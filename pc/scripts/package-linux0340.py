"""Stage an unpublished Linux 0.34.0 candidate from a strict, ROM-free whitelist.

Run on Linux after Root's native build and the self-contained Workshop publish.
This script never launches the game, changes a source profile, or overwrites an
export. The only subprocesses inspect trusted ELF inputs, patch staged RPATHs,
and inspect local package licences. Recipient machines need no build tools.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import subprocess
import sys
import tarfile
from datetime import datetime, timezone

PC = Path(__file__).resolve().parents[1]
ROOT = PC.parent
FRESH_SHA = "ccca4870f3b5f2d11519568c1f003387a62f5d13adbed9ed99c8089b74c68585"
ROM_SHA = "e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9"
CONTENT_SHA = "d5800ba53ceceb261048d72289805ed531e0f708916546febf7a41c22cdee87d"
ROM_MAGICS = {bytes.fromhex(x) for x in ("80371240", "37804012", "40123780")}
EXECUTABLES = ("Jouer.sh", "Atelier.sh", "pc/build/native-special/DinoTale",
               "pc/build/native-special/MusicRender0310",
               "pc/build/workshop-linux/YoshiStory-AtelierLinux")
PROFILE_FILES = {"saves/yoshisstory.n64.us.bin", "menu-progress.json", "general.json",
                 "graphics.json", "sound.json", "portable.txt", "book-menu.json"}
ROOT_FILES = {"Jouer.sh", "Atelier.sh", "LIRE-MOI.txt", "README.txt", "contenu-sha256.json"}
PC_FILES = {"portable-package.json", "upstream-lock.json", "linux-runtime-libraries.json",
            "distribution/content.bps", "distribution/content-patch.json",
            "save-imports/content-default.bin", "save-imports/content-import.json",
            "build/native-special/DinoTale", "build/native-special/MusicRender0310",
            "build/native-special/engine-build.json",
            "build/workshop-linux/YoshiStory-AtelierLinux"}
SYSTEM_LIBS = {"libc.so.6", "libm.so.6", "libpthread.so.0", "libdl.so.2", "librt.so.1",
               "libresolv.so.2", "libutil.so.1", "libanl.so.1", "libBrokenLocale.so.1"}
PRIVATE_LIBS = {"libSDL2-2.0.so.0"}
DRIVER_PREFIXES = ("libvulkan.", "libGL.", "libEGL.", "libGLX", "libGLdispatch.",
                   "libOpenGL.", "libGLES", "libdrm", "libnvidia", "libcuda", "libamd", "libvdpau")


def need(value, message):
    if not value:
        raise ValueError(message)


def sha(path):
    h = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def read(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def relative_name(value):
    need(isinstance(value, str) and value and "\\" not in value and "\x00" not in value,
         "Invalid archive path")
    p = PurePosixPath(value)
    need(not p.is_absolute() and ":" not in value and
         all(part not in ("", ".", "..") for part in value.split("/")), "Unsafe archive path: " + value)
    return p


def allowed_file(value):
    relative_name(value)
    if value in ROOT_FILES:
        return True
    if value.startswith("Licences/"):
        return bool(re.fullmatch(r"Licences/[A-Za-z0-9._+-]+\.txt", value))
    if not value.startswith("pc/"):
        return False
    name = value[3:]
    if name in PC_FILES:
        return True
    prefix = "profiles/content-personal/"
    if name.startswith(prefix):
        return name[len(prefix):] in PROFILE_FILES
    if name.startswith("build/native-special/lib/"):
        return bool(re.fullmatch(r"build/native-special/lib/lib[A-Za-z0-9_+.-]+\.so(?:\.[A-Za-z0-9_.+-]+)*", name))
    if name.startswith("build/native-special/assets/"):
        suffix = PurePosixPath(name).suffix.lower()
        return suffix in (".ttf", ".txt", ".md", ".rcss", ".svg", ".png")
    if name.startswith("assets/editor/"):
        return bool(re.fullmatch(r"assets/editor/(?:peche|mure|bleuet|poire-verte|prune|kiwi)-24x21\.png", name))
    return False


def inspect_file(path, name):
    need(allowed_file(name), "Unapproved package file: " + name)
    need(path.is_file() and not path.is_symlink(), "Only regular files are packaged: " + name)
    with path.open("rb") as stream:
        magic = stream.read(4)
    need(magic not in ROM_MAGICS, "Nintendo ROM bytes in package: " + name)
    need(path.suffix.lower() not in (".n64", ".z64", ".v64", ".eep", ".rdram", ".wav", ".pcm", ".bak", ".dmp", ".log"),
         "Private/generated content in package: " + name)


def elf64(path):
    with Path(path).open("rb") as stream:
        header = stream.read(20)
    need(len(header) == 20 and header[:6] == b"\x7fELF\x02\x01" and
         struct.unpack_from("<H", header, 18)[0] == 62, "Not a Linux x86-64 ELF: " + str(path))


def command(argv, env=None):
    result = subprocess.run([str(a) for a in argv], text=True, capture_output=True,
                            env=env, timeout=60, check=False)
    need(result.returncode == 0, "Command failed: " + str(argv[0]) + "\n" + result.stdout + result.stderr)
    return result.stdout


def system_library(name):
    # Mint22.3 owns its coherent desktop, C++ toolchain, TLS and GPU stack.
    # Only this explicitly reviewed SDL build is bundled in this candidate.
    return name not in PRIVATE_LIBS


def ldd(path, env=None):
    result = subprocess.run(["ldd", str(path)], text=True, capture_output=True, env=env, timeout=30)
    text = result.stdout + result.stderr
    if result.returncode and ("not a dynamic executable" in text or "statically linked" in text):
        return {}
    need(result.returncode == 0 and "not found" not in text, "Unresolved ELF dependency for " + str(path) + "\n" + text)
    dependencies = {}
    for line in text.splitlines():
        match = re.match(r"\s*(\S+)\s+=>\s+(/.*?)\s+\(0x[0-9a-fA-F]+\)\s*$", line)
        if match:
            name, resolved = match.groups()
            need(Path(resolved).is_file(), "Missing dependency " + resolved)
            dependencies[name] = Path(resolved)
    return dependencies


def library_closure(executables):
    pending = list(executables)
    # .NET loads OpenSSL dynamically; it is not necessarily visible in apphost ldd.
    available = command(["ldconfig", "-p"])
    for name in ("libssl.so.3", "libcrypto.so.3"):
        match = re.search(r"\b" + re.escape(name) + r"\s+\([^\n]*x86-64[^\n]*\)\s+=>\s+(/[^\n]+)", available)
        need(match, "Missing .NET native dependency: " + name)
        pending.append(Path(match.group(1).strip()))
    visited, bundle, system = set(), {}, {}
    while pending:
        path = pending.pop(0).resolve()
        if path in visited:
            continue
        visited.add(path)
        dependencies = ldd(path)
        if path.name in ("libssl.so.3", "libcrypto.so.3"):
            dependencies[path.name] = path
        for name, source in dependencies.items():
            if system_library(name):
                system[name] = "system graphics driver/loader" if name.startswith(DRIVER_PREFIXES) else "system desktop/runtime library"
                continue
            need(re.fullmatch(r"lib[A-Za-z0-9_+.-]+\.so(?:\.[A-Za-z0-9_.+-]+)*", name), "Unexpected shared library name: " + name)
            resolved = source.resolve()
            if name in bundle:
                need(sha(bundle[name]) == sha(resolved), "Conflicting ELF dependency: " + name)
            else:
                bundle[name] = resolved
                pending.append(resolved)
    need("libSDL2-2.0.so.0" in bundle, "The game must include its verified SDL2 runtime")
    return bundle, system


def copy(source, destination, inputs):
    source = Path(source)
    need(source.is_file(), "Missing source: " + str(source))
    inputs[str(source.resolve())] = sha(source)
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source, destination)
    os.chmod(destination, 0o644)


def notice_for(path):
    candidates = [str(path), str(path.resolve())]
    if str(path).startswith("/usr/lib/"):
        candidates.append(str(path).replace("/usr/lib/", "/lib/", 1))
    for candidate in dict.fromkeys(candidates):
        owner = subprocess.run(["dpkg-query", "-S", candidate], text=True, capture_output=True, timeout=20)
        if owner.returncode:
            continue
        package = owner.stdout.splitlines()[0].rsplit(": ", 1)[0].split(":", 1)[0]
        notice = Path("/usr/share/doc") / package / "copyright"
        if notice.is_file():
            return package, notice
    return None


def dotnet_notice(name, workshop):
    roots = [workshop.parent, Path(os.environ.get("DOTNET_ROOT", "/usr/share/dotnet")),
             Path("/usr/lib/dotnet"), Path.home() / ".dotnet", Path("/mnt/c/Program Files/dotnet")]
    variants = ("LICENSE.txt", "LICENSE.TXT") if name == "license" else ("ThirdPartyNotices.txt", "THIRD-PARTY-NOTICES.TXT", "THIRD-PARTY-NOTICES.txt")
    for directory in roots:
        for variant in variants:
            path = directory / variant
            if path.is_file():
                return path
    raise ValueError("Missing .NET licence/notice: " + name)


def licences(folder, dependencies, workshop, inputs):
    origins = {
        "DinoTale-GPL": "COPYING", "RT64": "lib/rt64/LICENSE", "N64ModernRuntime": "lib/N64ModernRuntime/COPYING",
        "N64Recomp": "lib/N64ModernRuntime/N64Recomp/LICENSE", "plume": "lib/rt64/src/contrib/plume/LICENSE",
        "hlslpp": "lib/rt64/src/contrib/hlslpp/LICENSE", "imgui": "lib/rt64/src/contrib/imgui/LICENSE.txt",
        "implot": "lib/rt64/src/contrib/implot/LICENSE", "im3d": "lib/rt64/src/contrib/im3d/LICENSE",
        "ddspp": "lib/rt64/src/contrib/ddspp/LICENSE", "nativefiledialog": "lib/rt64/src/contrib/nativefiledialog-extended/LICENSE",
        "stb": "lib/rt64/src/contrib/stb/LICENSE", "xxHash": "lib/rt64/src/contrib/xxHash/LICENSE",
        "zstd": "lib/rt64/src/contrib/zstd/LICENSE", "re-spirv": "lib/rt64/src/contrib/re-spirv/LICENSE",
        "miniz": "lib/N64ModernRuntime/thirdparty/miniz/LICENSE", "o1heap": "lib/N64ModernRuntime/thirdparty/o1heap/LICENSE",
        "RmlUi": "lib/RecompFrontend/recompui/lib/RmlUi/LICENSE.txt", "lunasvg": "lib/RecompFrontend/recompui/lib/lunasvg/LICENSE",
        "plutovg": "lib/RecompFrontend/recompui/lib/lunasvg/plutovg/LICENSE", "GamepadMotionHelpers": "lib/RecompFrontend/lib/GamepadMotionHelpers/LICENSE",
        "SlotMap": "lib/RecompFrontend/lib/SlotMap/slot_map.h", "LZMA-SDK": "thirdparty/lzma-sdk/lzma-sdk.txt",
        "screen-filters": "licenses/screen-filters.txt"}
    for name, relative in origins.items():
        copy(PC / "dinotale" / relative, folder / "Licences" / (name + ".txt"), inputs)
    copy(PC / "tools/linux-sources/SDL2-2.30.3/LICENSE.txt", folder / "Licences/SDL2-2.30.3.txt", inputs)
    copy(dotnet_notice("license", workshop), folder / "Licences/DotNET.txt", inputs)
    copy(dotnet_notice("third-party", workshop), folder / "Licences/DotNET-ThirdParty.txt", inputs)
    found = {}
    for soname, source in dependencies.items():
        entry = notice_for(source)
        if entry:
            package, notice = entry
            target = "Licences/Linux-" + package + ".txt"
            if target not in found:
                copy(notice, folder / target, inputs)
            found[target] = True
            found[soname] = target
        elif soname == "libSDL2-2.0.so.0":
            found[soname] = "Licences/SDL2-2.30.3.txt"
        else:
            raise ValueError("Cannot identify licence for " + soname + ": " + str(source))
    (folder / "Licences/Sources.txt").write_text(
        "DinoTale Tornie — local unpublished Linux candidate.\n"
        "Project: https://github.com/Torneko/dinotale-tornie\n"
        "Upstream: https://github.com/wootbeer/dinotale/tree/65060b54925016f634b4e0e327878f72051cd4a9\n"
        "Corresponding source and pinned submodule changes must accompany any future public release.\n"
        "This candidate is not a public release. Nintendo game images are not included.\n", encoding="utf-8")
    return {name: found[name] for name in dependencies}


def stage(args):
    need(sys.platform.startswith("linux"), "Create the tar.gz under Linux to preserve executable permissions")
    need(not args.output.exists(), "Existing candidate folders are never overwritten")
    runtime = args.runtime.resolve()
    workshop = args.workshop.resolve() / "YoshiStory-AtelierLinux"
    game, worker = runtime / "DinoTale", runtime / "MusicRender0310"
    native_content = runtime / "workshop-content.bin"
    need(native_content.is_file() and sha(native_content) == CONTENT_SHA,
         "Native build content differs from the approved BPS target; it must never be copied into the archive")
    for executable in (game, worker, workshop):
        elf64(executable)
    publish_manifest = args.workshop.resolve() / "publish-manifest0340.json"
    deployment = read(publish_manifest)
    need(deployment.get("exeSha256") == sha(workshop) and deployment.get("runtimeIdentifier") == "linux-x64" and
         all(deployment.get(key) is True for key in ("selfContained", "publishSingleFile", "includeNativeLibrariesForSelfExtract")),
         "Workshop must be the verified self-contained Linux single-file publish")
    deployment = {key: deployment[key] for key in ("exeSha256", "runtimeIdentifier", "selfContained", "publishSingleFile", "includeNativeLibrariesForSelfExtract")}
    capabilities = read(args.capabilities)
    need(capabilities.get("exeSha256") == sha(game) and
         capabilities.get("workerSha256", capabilities.get("menuMusicWorkerSha256")) == sha(worker) and
         capabilities.get("platform") == "Linux x86-64", "Native build metadata is not bound to these Linux ELF inputs")
    need(capabilities.get("contentSha256") == CONTENT_SHA and capabilities.get("customLevelVersion") == 6 and
         capabilities.get("soloYoshiColors") == 16 and capabilities.get("storyHard") is True,
         "Capability source does not describe the final 0.33.0 solo rules")
    seed = PC / "profiles/portable-fresh-seed-20261028/saves/yoshisstory.n64.us.bin"
    need(seed.stat().st_size == 2048 and sha(seed) == FRESH_SHA, "The fresh EEPROM seed changed")
    distribution = PC / "build/publication-0150/distribution"
    old_patch = read(distribution / "content-patch.json")
    need(old_patch.get("sourceSha256") == ROM_SHA and old_patch.get("targetSha256") == CONTENT_SHA and
         old_patch.get("sourceSize") == old_patch.get("targetSize") == 16777216 and
         old_patch.get("patchSha256") == sha(distribution / "content.bps"), "Historical BPS identity mismatch")
    need((distribution / "content.bps").read_bytes()[:4] == b"BPS1", "Invalid BPS source")
    dependencies, system = library_closure((game, worker, workshop))
    controlled_sdl = PC / "tools/sdl2-linux/lib/libSDL2-2.0.so.0"
    need(controlled_sdl.is_file() and sha(dependencies["libSDL2-2.0.so.0"]) == sha(controlled_sdl),
         "ELF resolves a different SDL than the controlled SDL2 build")
    args.output.mkdir(parents=True)
    folder = args.output / ("DinoTale-Tornie-" + args.version + "-Linux-x64-Candidate")
    folder.mkdir()
    inputs = {str(args.capabilities.resolve()): sha(args.capabilities), str((distribution / "content-patch.json").resolve()): sha(distribution / "content-patch.json"), str(publish_manifest): sha(publish_manifest), str(native_content): sha(native_content), str(Path(__file__).resolve()): sha(Path(__file__))}
    engine = folder / "pc/build/native-special"
    for origin in (game, worker):
        copy(origin, engine / origin.name, inputs)
        os.chmod(engine / origin.name, 0o755)
        command(["patchelf", "--set-rpath", "$ORIGIN/lib", engine / origin.name])
    atelier = folder / "pc/build/workshop-linux/YoshiStory-AtelierLinux"
    copy(workshop, atelier, inputs)
    os.chmod(atelier, 0o755)
    # Preserve the appended .NET single-file bundle byte for byte; bootstrap supplies LD_LIBRARY_PATH.
    for name, origin in dependencies.items():
        copy(origin, engine / "lib" / name, inputs)
    source_assets = runtime / "assets"
    need(source_assets.is_dir(), "Runtime assets missing")
    for origin in sorted(source_assets.rglob("*")):
        need(not origin.is_symlink(), "Runtime asset symlinks are not exported")
        if origin.is_file():
            copy(origin, engine / "assets" / origin.relative_to(source_assets), inputs)
    for name in ("peche", "mure", "bleuet", "poire-verte", "prune", "kiwi"):
        copy(PC / ("assets/editor/" + name + "-24x21.png"), folder / ("pc/assets/editor/" + name + "-24x21.png"), inputs)
    copy(PC / "upstream-lock.json", folder / "pc/upstream-lock.json", inputs)
    copy(distribution / "content.bps", folder / "pc/distribution/content.bps", inputs)
    profile = folder / "pc/profiles/content-personal"
    copy(seed, profile / "saves/yoshisstory.n64.us.bin", inputs)
    copy(seed, folder / "pc/save-imports/content-default.bin", inputs)
    write(folder / "pc/save-imports/content-import.json", {"seedSha256": FRESH_SHA, "testUnlocksUsed": False,
          "earnedUnlocksPreserved": True, "earnedYoshis": []})
    write(profile / "menu-progress.json", {"version": 1, "valid": True, "sourceSaveSha256": FRESH_SHA,
          "storyMask": 0, "trialMask": 0, "extraYoshiMask": 0, "trialEggs": 0})
    write(profile / "general.json", {"background_input_mode": "On", "debug_mode": False, "joystick_deadzone": 5, "rumble_strength": 25})
    write(profile / "graphics.json", {"api_option": "Vulkan", "ar_option": "Original", "developer_mode": False,
          "ds_option": 0, "hpfb_option": "Off", "hr_option": "Clamp16x9", "hybrid_dimming": 88,
          "msaa_option": "None", "res_option": "Original", "rr_manual_value": 60,
          "rr_option": "Original", "screen_filter": "None", "wm_option": "Windowed"})
    write(profile / "sound.json", {"main_volume": 3})
    write(profile / "book-menu.json", {"version": 1, "language": "fr"})
    (profile / "portable.txt").write_text("", encoding="utf-8")
    capabilities.update(version=args.version, platform="Linux x86-64", exeSha256=sha(engine / "DinoTale"),
                        menuMusicWorker="MusicRender0310", menuMusicWorkerSha256=sha(engine / "MusicRender0310"),
                        workerSha256=sha(engine / "MusicRender0310"),
                        manualUserAccepted=False, releaseReady=False, originalRomRequired=True, runtimeValidation="not-validated",
                        nativeBuildVerified=True, standaloneMusicCpuVerified=True, workshopApiVerified=True,
                        nativeLevelEntryVerified=False, softwareGraphicsValidation="failed-intro-stall-11-or-13-native-frames",
                        physicalLinuxMintValidation="pending",
                        validationScope="Linux build, standalone music CPU and Workshop API verified. WSL software graphical trials stalled during the intro at native frame11/13 with no playable Yoshi; gameplay is not validated. Physical Mint and grouped human tests remain pending.")
    write(engine / "engine-build.json", capabilities)
    patch = dict(old_patch, version=args.version, engineSha256=sha(engine / "DinoTale"))
    write(folder / "pc/distribution/content-patch.json", patch)
    notice_map = licences(folder, dependencies, workshop, inputs)
    write(folder / "pc/linux-runtime-libraries.json", {"schema": 1, "platform": "Linux x86-64",
          "system": system, "bundled": {name: {"sha256": sha(engine / "lib" / name), "notice": notice_map[name]} for name in sorted(dependencies)},
          "minimumGraphicsApi": "Vulkan 1.2", "targetDistribution": "Linux Mint 22.3",
          "bundlePolicy": "verified SDL2 only; desktop, TLS, C++ toolchain and drivers stay system-owned",
          "glibcMaximumAllowed": "2.39", "embeddedDotnetNativeClosureRequiresRuntimeTest": True})
    write(folder / "pc/portable-package.json", {"format": 1, "version": args.version, "platform": "Linux x86-64",
          "freshSave": True, "personalProgressIncluded": False, "testUnlocksForced": False,
          "volumePercent": 3, "originalRomRequired": True, "romIncluded": False,
          "saveFolder": "pc/profiles/content-personal", "workshopInterface": "local browser",
          "profileAssetsBootstrap": "runtime-to-profile-before-game",
          "engineSha256": sha(engine / "DinoTale"), "workshopSha256": sha(atelier), "workshopDeployment": deployment,
          "menuMusicWorkerSha256": sha(engine / "MusicRender0310"), "releaseReady": False,
          "runtimeValidation": "not-validated", "coopAndBossBattleDeferred": True,
          "nativeBuildVerified": True, "standaloneMusicCpuVerified": True, "workshopApiVerified": True,
          "nativeLevelEntryVerified": False, "softwareGraphicsValidation": "failed-intro-stall-11-or-13-native-frames",
          "physicalLinuxMintValidation": "pending"})
    for name, arguments in (("Jouer.sh", ' --play "$@"'), ("Atelier.sh", ' "$@"')):
        launcher = '''#!/bin/sh
set -eu
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export DOTNET_BUNDLE_EXTRACT_BASE_DIR="$base/pc/cache/dotnet"
export LD_LIBRARY_PATH="$base/pc/build/native-special/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$base/pc/build/workshop-linux/YoshiStory-AtelierLinux" "$base/pc"'''
        (folder / name).write_text(launcher + arguments + "\n", encoding="utf-8")
        os.chmod(folder / name, 0o755)
    (folder / "LIRE-MOI.txt").write_text(
        "DinoTale Tornie " + args.version + " — Linux x86-64 — candidat local non publié\n\n"
        "Extraire le dossier complet puis ouvrir Jouer.sh (ou ./Jouer.sh dans un terminal).\n"
        "Au premier lancement, sélectionner une ROM originale USA de Yoshi’s Story.\n"
        "Les formats .z64, .v64 et .n64 sont reconnus. Le traitement reste local.\n"
        "Aucune ROM, progression personnelle ni cache musical n’est fourni.\n"
        "Nouvelle sauvegarde, aucune monnaie ni record. Volume initial : 3 %.\n"
        "Atelier.sh ouvre l’éditeur local dans le navigateur installé, sans connexion Internet.\n"
        "Conserver tout pc/profiles/content-personal pour sauvegarder la progression ; créations dans pc/content.\n\n"
        "Cible : Linux Mint 22.3, PC x86-64 avec processeur de classe Nehalem ou plus récent,\n"
        "pilote Vulkan 1.2 et session de bureau. SDL2 est fournie ; glibc, bibliothèques de bureau,\n"
        "OpenSSL3 et bibliothèques C++ restent celles du système Mint.\n"
        "Aucun émulateur, Python, compilateur ou installation .NET n’est nécessaire.\n"
        "La bibliothèque de l’Atelier est autonome ; un navigateur et les services de bureau restent nécessaires.\n\n"
        "Compilation native, moteur musical CPU et API Atelier vérifiés.\n"
        "Le rendu logiciel sous WSL reste bloqué à l’introduction (11/13 images natives),\n"
        "sans Yoshi jouable. La jouabilité Linux n’est pas validée ; le vrai PC Mint reste à tester.\n"
        "Candidat non final : coop et Boss Battle restent différés.\n"
        "Les essais physiques Mint, manettes et parcours longs restent distincts des contrôles automatisés.\n"
        "Ne pas diffuser comme version finale ; ce paquet ne publie rien sur GitHub ou le site.\n", encoding="utf-8")
    (folder / "README.txt").write_text(
        "DinoTale Tornie " + args.version + " — Linux x86-64 — unpublished local candidate\n\n"
        "Extract the complete folder and run ./Jouer.sh. On first launch select an original\n"
        "USA Yoshi’s Story ROM (.z64, .v64 or .n64). Import and content preparation are local.\n"
        "No ROM, personal progress or music cache is distributed. Fresh save; initial volume 3%.\n"
        "Atelier.sh opens the offline local level editor in the installed browser.\n"
        "Back up pc/profiles/content-personal; creations are stored in pc/content.\n"
        "Target: Linux Mint 22.3 x86-64, Nehalem-class CPU or newer, Vulkan 1.2 driver.\n"
        "No emulator, Python, compiler or .NET installation is required. System graphics\n"
        "drivers, glibc, desktop/C++ libraries, OpenSSL3 and a browser are still required.\n"
        "Native build, standalone music CPU renderer and Workshop API checks passed.\n"
        "WSL software graphics stalled during the intro (native frame11/13), with no playable Yoshi.\n"
        "Linux gameplay is not validated; actual Mint hardware testing remains pending.\n"
        "Unfinished candidate: co-op and Boss Battle remain deferred. This is not a public release.\n", encoding="utf-8")
    inventory = {}
    for path in sorted(folder.rglob("*")):
        need(not path.is_symlink(), "Package contains a symlink")
        if path.is_file():
            name = path.relative_to(folder).as_posix()
            inspect_file(path, name)
            inventory[name] = sha(path)
    write(folder / "contenu-sha256.json", inventory)
    need(all(sha(Path(path)) == digest for path, digest in inputs.items()), "A source changed while staging")
    archive = args.output / (folder.name + ".tar.gz")
    with tarfile.open(archive, "x:gz", compresslevel=6) as tar:
        def metadata(info):
            info.uid = info.gid = 0
            info.uname = info.gname = ""
            info.mtime = 0
            info.mode = 0o755 if info.isdir() or info.name.removeprefix(folder.name + "/") in EXECUTABLES else 0o644
            return info
        tar.add(folder, arcname=folder.name, filter=metadata)
    report = {"schema": 1, "version": args.version, "platform": "Linux x86-64", "candidateOnly": True,
              "folder": str(folder), "archive": str(archive), "archiveSha256": sha(archive),
              "bytes": archive.stat().st_size, "fileCount": len(inventory),
              "romIncluded": False, "personalProfileAccessed": False, "freshSaveSha256": FRESH_SHA,
              "volumePercent": 3, "engineSha256": sha(engine / "DinoTale"),
              "workerSha256": sha(engine / "MusicRender0310"), "workshopSha256": sha(atelier),
              "inputFilesUnchanged": True, "inputFiles": inputs,
              "bundledLibraries": sorted(dependencies), "requiredSystemLibraries": sorted(system),
              "runtimeValidation": "not-validated", "hardwareMintValidation": "pending",
              "nativeBuildVerified": True, "standaloneMusicCpuVerified": True, "workshopApiVerified": True,
              "nativeLevelEntryVerified": False, "softwareGraphicsValidation": "failed-intro-stall-11-or-13-native-frames",
              "createdUtc": datetime.now(timezone.utc).isoformat()}
    write(args.output / "package-report.json", report)
    print(json.dumps({k: report[k] for k in ("folder", "archive", "archiveSha256", "fileCount", "runtimeValidation")}, indent=2))


def public_prerelease(args):
    # Repackage only reviewed Candidate01; never spawn a process or rewrite ELF.
    import importlib.util
    sys.dont_write_bytecode = True
    need(args.version == "0.34.0" and not args.output.exists(), "Wrong version or existing prerelease directory")
    source = args.public_prerelease_from.resolve()
    expected = ROOT / "dist/Linux-0.34.0-Candidate-01/DinoTale-Tornie-0.34.0-Linux-x64-Candidate"
    need(source == expected.resolve(), "Only reviewed Candidate01 may be repackaged")
    original_archive = source.parent / (source.name + ".tar.gz")
    original_sha = "706698f54b396c85a9b0f44a0096237b119cbdac2d23b9db48d2011a4416e5e9"
    need(sha(original_archive) == original_sha, "Candidate01 archive changed")
    spec = importlib.util.spec_from_file_location("linux_package0340_public_check", Path(__file__).with_name("check-linux-package0340.py"))
    check = importlib.util.module_from_spec(spec); sys.modules[spec.name] = check; spec.loader.exec_module(check)
    need(check.validate(source, static_only=True)["passed"], "Candidate static checks failed")
    before = {p.relative_to(source).as_posix(): sha(p) for p in source.rglob("*") if p.is_file()}
    args.output.mkdir(parents=True)
    version = "0.34.0-beta.1"; tag = "v" + version
    name = "DinoTale-Tornie-" + version + "-Linux-x64"
    folder = args.output / name
    source_url = "https://github.com/Torneko/dinotale-tornie/tree/" + tag
    source_asset = "DinoTale-Tornie-" + version + "-Sources.zip"
    source_archive_url = "https://github.com/Torneko/dinotale-tornie/releases/download/" + tag + "/" + source_asset
    for relative in sorted(before):
        origin, target = source / relative, folder / relative
        inspect_file(origin, relative); target.parent.mkdir(parents=True, exist_ok=True); shutil.copyfile(origin, target)
    text = (folder / "LIRE-MOI.txt").read_text(encoding="utf-8")
    text = text.replace("DinoTale Tornie 0.34.0 — Linux x86-64 — candidat local non publié",
                        "DinoTale Tornie " + version + " — Linux x86-64 — préversion publique expérimentale")
    text = text.replace("Ne pas diffuser comme version finale ; ce paquet ne publie rien sur GitHub ou le site.",
                        "Cette préversion expérimentale n’est pas une version finale.")
    text += "\nSources correspondantes : " + source_url + "\nArchive des sources : " + source_archive_url + "\n"
    (folder / "LIRE-MOI.txt").write_text(text, encoding="utf-8")
    text = (folder / "README.txt").read_text(encoding="utf-8")
    text = text.replace("DinoTale Tornie 0.34.0 — Linux x86-64 — unpublished local candidate",
                        "DinoTale Tornie " + version + " — Linux x86-64 — experimental public prerelease")
    text = text.replace("This is not a public release.", "This experimental prerelease is not a final release.")
    text += "\nCorresponding source: " + source_url + "\nSource archive: " + source_archive_url + "\n"
    (folder / "README.txt").write_text(text, encoding="utf-8")
    (folder / "Licences/Sources.txt").write_text(
        "DinoTale Tornie " + version + " — experimental public Linux prerelease.\n"
        "Corresponding source tag: " + source_url + "\nCorresponding source archive: " + source_archive_url + "\n"
        "Matching source, build/import/package scripts and pinned library changes are provided through these links.\n"
        "Upstream: https://github.com/wootbeer/dinotale/tree/65060b54925016f634b4e0e327878f72051cd4a9\n"
        "Nintendo game images are not included.\n", encoding="utf-8")
    for relative in ("pc/portable-package.json", "pc/build/native-special/engine-build.json"):
        meta = read(folder / relative)
        if relative == "pc/portable-package.json": meta["version"] = version
        meta.update(distributionStatus="experimental-public-prerelease", publicReleaseTag=tag,
                    correspondingSource=source_url, correspondingSourceArchive=source_archive_url,
                    releaseReady=False, manualUserAccepted=False, runtimeValidation="not-validated",
                    nativeLevelEntryVerified=False, physicalLinuxMintValidation="pending",
                    softwareGraphicsValidation="failed-intro-stall-11-or-13-native-frames")
        write(folder / relative, meta)
    changed_allowed = {"LIRE-MOI.txt", "README.txt", "Licences/Sources.txt", "pc/portable-package.json",
                       "pc/build/native-special/engine-build.json", "contenu-sha256.json"}
    inventory = {}
    for p in sorted(folder.rglob("*")):
        need(not p.is_symlink(), "Prerelease contains a symlink")
        if not p.is_file() or p.name == "contenu-sha256.json": continue
        relative = p.relative_to(folder).as_posix(); inspect_file(p, relative); inventory[relative] = sha(p)
        if relative not in changed_allowed: need(inventory[relative] == before[relative], "Unexpected content change: " + relative)
    need(set(inventory) == set(before) - {"contenu-sha256.json"}, "Prerelease whitelist changed")
    write(folder / "contenu-sha256.json", inventory)
    verification = check.validate(folder, static_only=True)
    need(verification["passed"], "Prerelease static checks failed")
    for relative in ("LIRE-MOI.txt", "README.txt", "Licences/Sources.txt"):
        text = (folder / relative).read_text(encoding="utf-8")
        need(source_url in text and source_archive_url in text and
             "candidat local non publié" not in text and "not a public release" not in text and
             "unpublished local candidate" not in text, "Incorrect prerelease documentation")
    archive = args.output / (name + ".tar.gz")
    with tarfile.open(archive, "x:gz", compresslevel=6) as tar:
        def metadata(info):
            info.uid = info.gid = 0; info.uname = info.gname = ""; info.mtime = 0
            info.mode = 0o755 if info.isdir() or info.name.removeprefix(name + "/") in EXECUTABLES else 0o644
            return info
        tar.add(folder, arcname=name, filter=metadata)
    with tarfile.open(archive, "r:gz") as tar:
        entries, top = check.members(tar); need(top == name, "Incorrect archive root")
        for relative, digest in inventory.items():
            stream = tar.extractfile(name + "/" + relative)
            need(stream is not None and hashlib.sha256(stream.read()).hexdigest() == digest, "Archived file differs: " + relative)
        need(json.load(tar.extractfile(name + "/contenu-sha256.json")) == inventory, "Archived inventory differs")
        for relative in EXECUTABLES:
            need(tar.getmember(name + "/" + relative).mode == 0o755, "Executable mode lost: " + relative)
    need(sha(original_archive) == original_sha and all(sha(source / relative) == digest for relative, digest in before.items()),
         "Candidate01 changed while repackaging")
    exact = ("pc/build/native-special/DinoTale", "pc/build/native-special/MusicRender0310",
             "pc/build/workshop-linux/YoshiStory-AtelierLinux", "pc/distribution/content-patch.json", "pc/distribution/content.bps")
    report = {"passed": True, "version": version, "distributionStatus": "experimental-public-prerelease",
              "folder": str(folder), "archive": str(archive), "archiveSha256": sha(archive), "bytes": archive.stat().st_size,
              "fileCount": len(inventory), "romIncluded": False, "freshSaveSha256": FRESH_SHA, "volumePercent": 3,
              "candidate01Unchanged": True, "candidate01ArchiveSha256": original_sha,
              "binaryAndImportByteExact": {relative: sha(folder / relative) for relative in exact},
              "onlyDocumentationAndDistributionMetadataChanged": True,
              "correspondingSource": source_url, "correspondingSourceArchive": source_archive_url,
              "archiveHashesAndUnixPermissionsVerified": True, "nativeGameLaunched": False, "WSLStarted": False, "GPUUsed": False,
              "runtimeValidation": "not-validated", "nativeLevelEntryVerified": False, "physicalLinuxMintValidation": "pending",
              "softwareGraphicsValidation": "failed-intro-stall-11-or-13-native-frames", "staticCheck": verification,
              "sourceScriptSha256": sha(Path(__file__)), "createdUtc": datetime.now(timezone.utc).isoformat()}
    write(args.output / "package-report.json", report)
    print(json.dumps({key: report[key] for key in ("passed", "archive", "archiveSha256", "bytes", "fileCount", "candidate01Unchanged", "runtimeValidation")}, indent=2))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--runtime", type=Path, default=PC / "build/native-linux-0340")
    p.add_argument("--workshop", type=Path, default=PC / "build/workshop-linux0340")
    p.add_argument("--capabilities", type=Path, default=PC / "build/native-linux-0340/engine-build.json")
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--version", default="0.34.0")
    p.add_argument("--public-prerelease-from", type=Path, help="Repackage Candidate01 without WSL or binary changes")
    p.add_argument("--plan", action="store_true", help="Only list required inputs; no tools or writes")
    args = p.parse_args()
    need(re.fullmatch(r"\d+\.\d+\.\d+", args.version), "Invalid candidate version")
    args.output = args.output.resolve()
    if args.public_prerelease_from:
        need(args.output.is_relative_to((ROOT / "dist").resolve()) and args.output.name.startswith("Linux-0.34.0-beta.1-"), "Only a new local prerelease dist directory is allowed")
        need(not args.plan, "Public conversion needs the reviewed candidate path")
        public_prerelease(args)
        return
    need(args.output.is_relative_to((ROOT / "dist").resolve()) and "Candidate" in args.output.name,
         "Only a new local dist candidate directory is allowed")
    if args.plan:
        required = [args.runtime / "DinoTale", args.runtime / "MusicRender0310", args.runtime / "assets", args.runtime / "workshop-content.bin",
                    args.workshop / "YoshiStory-AtelierLinux", args.workshop / "publish-manifest0340.json", args.capabilities,
                    PC / "profiles/portable-fresh-seed-20261028/saves/yoshisstory.n64.us.bin",
                    PC / "build/publication-0150/distribution/content.bps"]
        print(json.dumps({"required": {str(path): path.exists() for path in required}, "output": str(args.output),
                          "existingOutputRefused": args.output.exists(), "toolsRequired": ["ldd", "ldconfig", "patchelf", "dpkg-query"]}, indent=2))
        return
    stage(args)


if __name__ == "__main__":
    main()