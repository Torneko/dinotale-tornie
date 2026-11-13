"""Stage an unpublished Linux 0.35.5 candidate from a strict, ROM-free whitelist.

Run on a clean Ubuntu22.04 CI runner after the fresh native build and Workshop
publish. Only generated C/RSP source inputs are allowed; no original ROM or
prepared game content is read, uploaded or embedded by this packaging policy.
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
               "pc/build/workshop-linux/YoshiStory-AtelierLinux", "Bonus-en-test.sh")
PROFILE_FILES = {"saves/yoshisstory.n64.us.bin", "menu-progress.json", "general.json",
                 "graphics.json", "sound.json", "portable.txt", "book-menu.json"}
ROOT_FILES = {"Jouer.sh", "Atelier.sh", "Bonus-en-test.sh", "LIRE-MOI.txt", "README.txt", "SPOILERS-OPTIONNELS.txt", "contenu-sha256.json"}
PC_FILES = {"portable-package.json", "upstream-lock.json", "linux-runtime-libraries.json",
            "distribution/content.bps", "distribution/content-patch.json",
            "save-imports/content-default.bin", "save-imports/content-import.json",
            "build/native-special/DinoTale", "build/native-special/MusicRender0310",
            "build/native-special/engine-build.json",
            "build/workshop-linux/YoshiStory-AtelierLinux"}
SYSTEM_LIBS = {"libc.so.6", "libm.so.6", "libpthread.so.0", "libdl.so.2", "librt.so.1",
               "libresolv.so.2", "libutil.so.1", "libanl.so.1", "libBrokenLocale.so.1"}
PRIVATE_LIBS = {"libSDL2-2.0.so.0"}
GLIBC_BASELINE = (2, 35)  # Ubuntu22.04 build baseline; also compatible with newer Mint.
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
    # Mint 22.3 owns its coherent desktop, C++ toolchain, TLS and GPU stack.
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


def glibc_requirement(path):
    versions = command(["readelf", "--version-info", path])
    found = [tuple(map(int, item)) for item in re.findall(r"\bGLIBC_(\d+)\.(\d+)\b", versions)]
    maximum = max(found, default=(0, 0))
    need(maximum <= GLIBC_BASELINE, "ELF requires glibc newer than the Ubuntu22.04/2.35 baseline: " + str(path))
    return maximum


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


def guide_texts0355():
    return ('DinoTale Tornie 0.35.5 — Linux x86-64 — candidate privée\n\nExtraire le dossier complet dans un nouveau dossier, puis ouvrir ./Jouer.sh.\nAu premier lancement, sélectionner une ROM originale USA de Yoshi’s Story\n(.z64/.v64/.n64). La préparation reste locale. Cette archive ne contient aucune\nROM ni progression personnelle. Sauvegarde vierge, monnaies à zéro, volume initial 3 %.\nAtelier.sh ouvre l’éditeur autonome dans le navigateur local. Aucun .NET, Python,\ncompilateur ou émulateur à installer ; session de bureau, navigateur, bibliothèques\nsystème et pilote Vulkan 1.2 restent nécessaires. Cible : PC Linux 64 bits, Mint 22.3.\n\nCette version ajoute des découvertes et des objets pour les modes solo, corrige\nleurs récompenses et préserve les progrès existants. Les personnages non découverts\nrestent anonymes dans les menus et grisés dans l’illustration du livre.\nPour découvrir les nouveautés sans les révéler, ne pas ouvrir\nSPOILERS-OPTIONNELS.txt : ce fichier séparé contient les détails de déblocage.\nLa coop et le Boss Battle demandent encore des finitions et des essais groupés.\nLes contrôles de compilation/CPU/ELF/paquet ne remplacent pas des essais de jeu\nsur matériel Linux : aucun nouveau jeu, rendu GPU ou son n’a été lancé pour ce lot.\nCette archive privée ne publie rien sur GitHub ni sur le site.\n\nBonus-en-test.sh permet d’essayer le mode Bonus avec les droits déjà acquis.\nIl relit les droits à chaque ouverture, sans accorder de personnage, monnaie\nou victoire.\nUtiliser Jouer.sh une première fois pour importer la ROM et préparer le contenu.\nLe lanceur Bonus utilise des outils Linux standards (awk, sha256sum, flock, pgrep,\nfind, cmp). Sa copie pc/profiles/bonus0353-qa-linux conserve ses propres réussites,\nsans fusion inverse vers le profil normal. Fermer DinoTale avant de changer de\nlanceur. Les conditions détaillées figurent dans le guide facultatif.\n\nMigration depuis Linux0340/0354 : conserver une sauvegarde complète de l’ancienne\ncopie et extraire 0355 séparément. Transférer TOUT pc/profiles/content-personal\nvers la nouvelle copie pour garder EEPROM, journaux, records, monnaies et ROM\nimportée ; copier aussi pc/content pour les créations. Conserver également tout\npc/profiles/bonus0353-qa-linux s’il existe : sa progression est indépendante.\nNe jamais remplacer les anciennes données par le seed vierge ni recopier les\nanciens binaires. Les anciens journaux sont conservés et les victoires déjà\nacquises restent lisibles. Aucun déblocage n’est injecté.\n',
            'DinoTale Tornie 0.35.5 — Linux x86-64 — private candidate\n\nExtract the complete folder into a new directory and run ./Jouer.sh. On first\nlaunch select an original USA Yoshi’s Story ROM (.z64/.v64/.n64). Preparation\nstays local. No ROM or personal progression is included. Fresh save, currency 0,\ninitial volume 3%. Atelier.sh opens the self-contained editor in the local browser.\nNo .NET/Python/compiler/emulator installation is needed. A desktop session,\nbrowser, system libraries and Vulkan 1.2 driver remain necessary. Target: 64-bit\nLinux PCs, including Mint 22.3.\n\nNew solo discoveries and editor objects, corrected rewards and existing progress\nare supported. Undiscovered characters remain anonymous in menus and gray in the\nbook illustration. To discover them naturally, leave SPOILERS-OPTIONNELS.txt\nclosed: that separate optional guide reveals unlock details.\nCo-op and Boss Battle still need finishing and grouped gameplay checks.\nBuild/CPU/ELF/package checks do not certify real Linux gameplay: no new game,\nGPU rendering or audio session was run for this batch. This private archive does\nnot publish anything to GitHub or the website.\n\nBonus-en-test.sh previews Bonus using already earned normal-profile rights.\nEach launch refreshes rights without granting characters, currency or victories. First use Jouer.sh to\nimport the original ROM and prepare the content. Standard Linux tools are needed\n(awk, sha256sum, flock, pgrep, find, cmp). Bonus progress remains independent in\npc/profiles/bonus0353-qa-linux and is never merged back into the normal profile.\nClose DinoTale before switching launchers. Detailed conditions are in the\noptional guide.\n\nMigration from Linux0340/0354: keep a complete backup and extract 0355 separately.\nTransfer the COMPLETE pc/profiles/content-personal directory to retain EEPROM,\njournals, records, currency and imported ROM; also transfer pc/content creations.\nKeep the entire pc/profiles/bonus0353-qa-linux directory if present: Bonus progress\nis independent. Never overwrite old data with the supplied fresh seed or copy\nold binaries over the new ones. Old ledgers are preserved and already earned\nvictories remain readable. No unlocks are injected.\n',
            'ATTENTION SPOILERS / OPTIONAL DISCOVERY SPOILERS\nCe fichier est facultatif. Le guide principal ne révèle pas ces informations.\nThis file is optional. The main guide does not reveal this information.\n\nFR — Deux personnages élémentaires : Feu et Eau. Ils se trouvent uniquement en\nStory difficile, vers la fin des niveaux 2-2 et 3-4 respectivement. Transporter\nl’œuf puis terminer le niveau pour conserver la découverte. Les droits deviennent\nalors utilisables dans les modes solo, le Boss Battle et l’éditeur.\nL’accès au Story difficile reste fondé sur les 16 personnages historiques et leurs\n16 victoires Boss réelles : les deux découvertes n’ajoutent pas de cercle bloquant.\nFeu utilise les œufs du Noir et une langue noire ; il nage à la surface de la lave,\nles piments lui rendent sa vie et l’ennemi enflammé comestible donne 1 point/1 PV.\nEau utilise les œufs du Blanc et une langue cyan ; nage plus rapide, protection\ncontre les boules de neige, piments −3 PV. La vie des ennemis aquatiques comestibles\nest triplée sans tripler leurs points ; pingouins 2 points/2 PV. Les poissons à\npiquants restent immangeables. Ces comportements attendent les essais physiques.\n\nEN — Two elemental characters: Fire and Water. Their eggs appear only in Hard\nStory near the end of courses 2-2 and 3-4 respectively. Carry the egg and finish\nthe course to preserve the discovery, then use it in solo modes, Boss Battle and\nthe editor. Hard Story still requires the original 16 characters and their genuine\n16 Boss wins; the two discoveries are not additional entry prerequisites.\nFire uses Black’s egg behavior and a black tongue, floats/swims at lava level,\nheals from chili and gains 1 point/1 health from the edible flaming creature.\nWater uses White’s egg behavior and a cyan tongue, swims faster and resists\nsnowballs; chili removes 3 health. Edible aquatic enemies triple healing only,\nnot points; penguins give 2 points/2 health. Spiny fish stay inedible.\nThese powers still require physical gameplay validation.\n\nBONUS — Facultatif / optional:\nFR : Les six Bonus utilisent les vrais droits du profil normal : terminer les\nquatre Trial de la page, classiques ou hybrides. Pour certains pots partagés,\nmaintenir L à l’entrée permet de revenir en arrière.\nEN: The six Bonus routes use genuine normal-profile rights: finish the page’s\nfour Trial courses, classic or hybrid. A few shared pots go backward when L\nis held on entry and forward otherwise.\n')


def feature_contract(capabilities):
    need(capabilities.get("customLevelVersion") == 6 and capabilities.get("soloYoshiColors") == 18 and
         capabilities.get("customYoshiColors") == 18 and capabilities.get("bossYoshiColors") == 18 and
         capabilities.get("bossChallengeSchema", capabilities.get("bossChallengeLedgerSchema")) == 3 and
         capabilities.get("soloElementalYoshis") is True and capabilities.get("editorElementalPlacements") is True and
         capabilities.get("editorSpecialEggCount") == 12 and capabilities.get("elementalOwnershipJournalSchema") == 2 and
         capabilities.get("unknownYoshiMenuIdentitiesHidden") is True and capabilities.get("unknownYoshiIconsNotSelectable") is True and
         capabilities.get("titleArtworkDesaturatesUndiscoveredHybrids") is True and
         capabilities.get("storyHard") is True and capabilities.get("storyHardPublicGate") == "all16owned-and-genuineBoss16" and
         capabilities.get("storyHardElementalPrerequisite") is False and capabilities.get("bonusPublicEnabled") is False,
         "Capability source must describe genuine0355/18colors, Bossledger3, anti-spoiler UI and original 16 Hard prerequisites")


def deployment_contract(deployment, expected_hash):
    need(deployment.get("exeSha256") == expected_hash and deployment.get("runtimeIdentifier") == "linux-x64" and
         all(deployment.get(key) is True for key in ("selfContained", "publishSingleFile", "includeNativeLibrariesForSelfExtract")) and
         deployment.get("version") == "0.35.5" and deployment.get("soloYoshiColors") == 18 and
         deployment.get("editorSpecialEggCount") == 12 and deployment.get("editorElementalPlacements") is True and
         deployment.get("unknownYoshiMenuIdentitiesHidden") is True and
         deployment.get("nativeLinuxExecutionVerified") is True and deployment.get("workshopApiVerified") is True and
         isinstance(deployment.get("apiProof"), str) and re.fullmatch(r"[0-9a-f]{64}", deployment.get("apiProofSha256", "")),
         "Workshop must be the current0355 self-contained Linux publish with exact native headless API proof")
    relative_name(deployment["apiProof"])


def native_contract(proof, engine_hash, worker_hash, metadata_hash=None):
    need(all(isinstance(value, str) and re.fullmatch(r"[0-9a-f]{64}", value) for value in (engine_hash, worker_hash)), "Missing source ELF hashes in native proof")
    need(proof.get("passed") is True and proof.get("version") == "0.35.5" and
         proof.get("engineSha256") == engine_hash and proof.get("workerSha256") == worker_hash and
         proof.get("freshNativeBuild") is True and proof.get("inputsUnchanged") is True and
         proof.get("originalRomUsed") is False and proof.get("preparedContentUsed") is False,
         "A fresh no-ROM Linux0355 build proof bound to these exact ELF inputs is required")
    if metadata_hash is not None:
        need(proof.get("engineBuildMetadataSha256") == metadata_hash, "Build proof metadata hash mismatch")
    private = proof.get("privateInputs", {})
    need(private.get("generatedCount") == 219 and isinstance(private.get("manifestPath"), str) and
         all(re.fullmatch(r"[0-9a-f]{64}", private.get(key, "")) for key in
             ("bundlePlaintextSha256", "manifestSha256", "generatedManifestSha256", "rspSha256")),
         "Generated C/RSP private-input proof missing")
    relative_name(private["manifestPath"])
    content = proof.get("contentCompiled", {})
    need(content.get("expectedSha256") == CONTENT_SHA and content.get("fnv64") == "5E83770716DC3E31" and content.get("placeholderOnly") is True and
         content.get("realContentUploaded") is False, "Real prepared content must not be uploaded or compiled remotely")
    worker = proof.get("workerQualification", {})
    need(worker.get("identityExecuted") is True and worker.get("actualRenderExecuted") is False and
         worker.get("sourceIdentityVerified") is True and re.fullmatch(r"[0-9a-f]{64}", worker.get("rendererIdentity", "")) and
         isinstance(worker.get("priorAudioReference"), dict) and worker["priorAudioReference"].get("windows0355") and
         worker["priorAudioReference"].get("linux0354") and
         worker["priorAudioReference"].get("rendererIdentity") == worker["rendererIdentity"] and
         worker["priorAudioReference"].get("pcmSha256") == "950824e78db6b852ddf7ebb8903d19ea7e433a060b130fa85852d4363f3c49f1" and
         worker["priorAudioReference"].get("wavSha256") == "5f1b5e3b322eda0eb35e5d3c948e92e56f29a922f9e0f7af394665173c5565db",
         "CI qualifies worker source identity; a fresh ROM render must remain unclaimed")


def private_input_contract(manifest, native):
    private = native["privateInputs"]
    names = {"funcs_" + str(index) + ".c" for index in range(216)} | {"lookup.cpp", "funcs.h", "recomp_overlays.inl"}
    expected = {"RecompiledFuncs/" + name for name in names} | {"rsp/aspMain.cpp"}
    files = manifest.get("files", {})
    normalized = manifest.get("generatedNormalizedSha256", {})
    need(manifest.get("schema") == 1 and manifest.get("version") == "0.35.5" and
         manifest.get("originalRomIncluded") is False and manifest.get("preparedContentIncluded") is False and
         manifest.get("saveIncluded") is False and manifest.get("generatedCount") == 219 and
         manifest.get("contentSha256") == CONTENT_SHA and manifest.get("contentFnv64") == "5E83770716DC3E31",
         "Private-input manifest must describe generated sources only, without ROM, prepared content or saves")
    need(isinstance(files, dict) and set(files) == expected and isinstance(normalized, dict) and set(normalized) == names and
         all(isinstance(value, str) and re.fullmatch(r"[0-9a-f]{64}", value) for value in (*files.values(), *normalized.values())),
         "Private-input manifest has unexpected files or malformed hashes")
    canonical = json.dumps(normalized, sort_keys=True, separators=(",", ":")).encode()
    need(manifest.get("generatedManifestSha256") == private["generatedManifestSha256"] == hashlib.sha256(canonical).hexdigest() and
         manifest.get("rspSha256") == private["rspSha256"] == files["rsp/aspMain.cpp"], "Private generated C/RSP identity differs from native proof")


def qualification_contract(meta):
    need(isinstance(meta.get("nativeBuildProofSha256"), str) and re.fullmatch(r"[0-9a-f]{64}", meta["nativeBuildProofSha256"]), "Native build proof hash missing")
    need(meta.get("nativeBuildVerified") is True and meta.get("standaloneMusicCpuVerified") is False and
         meta.get("standaloneMusicSourceIdentityVerified") is True and meta.get("freshMusicRomRenderVerified") is False and
         meta.get("originalRomUsedForBuild") is False and meta.get("preparedContentUsedForBuild") is False,
         "No-ROM CI qualification must not claim a fresh worker ROM render")
    native_contract(meta.get("nativeBuildProvenance", {}), meta.get("sourceEngineSha256"), meta.get("sourceWorkerSha256"))


def stage(args):
    need(sys.platform.startswith("linux"), "Create the tar.gz under Linux to preserve executable permissions")
    need(not args.output.exists(), "Existing candidate folders are never overwritten")
    runtime = args.runtime.resolve()
    workshop = args.workshop.resolve() / "YoshiStory-AtelierLinux"
    game, worker = runtime / "DinoTale", runtime / "MusicRender0310"
    need(not (runtime / "workshop-content.bin").exists(), "Remove the CI placeholder content before packaging; real content is never a package input")
    for executable in (game, worker, workshop):
        elf64(executable)
    publish_manifest = args.workshop.resolve() / "publish-manifest0355.json"
    deployment = read(publish_manifest)
    deployment_contract(deployment, sha(workshop))
    api_proof = ROOT / deployment["apiProof"]
    need(api_proof.is_file() and sha(api_proof) == deployment["apiProofSha256"], "Workshop API proof hash mismatch")
    api_result = read(api_proof)
    need(api_result.get("passed") is True and api_result.get("appSha256") == sha(workshop), "Workshop native API proof is not bound to this ELF")
    deployment = {key: deployment[key] for key in ("exeSha256", "runtimeIdentifier", "selfContained", "publishSingleFile", "includeNativeLibrariesForSelfExtract", "version", "soloYoshiColors", "editorSpecialEggCount", "editorElementalPlacements", "unknownYoshiMenuIdentitiesHidden", "nativeLinuxExecutionVerified", "workshopApiVerified", "apiProof", "apiProofSha256")}
    capabilities = read(args.capabilities)
    need(capabilities.get("exeSha256") == sha(game) and
         capabilities.get("workerSha256", capabilities.get("menuMusicWorkerSha256")) == sha(worker) and
         capabilities.get("platform") in {"Linux x86-64", "Linux x64"}, "Native build metadata is not bound to these Linux ELF inputs")
    need(capabilities.get("contentSha256") == CONTENT_SHA, "Capability content differs")
    feature_contract(capabilities)
    build_proof = args.build_proof.resolve()
    native = read(build_proof)
    native_contract(native, sha(game), sha(worker), sha(args.capabilities))
    input_manifest = ROOT / native["privateInputs"]["manifestPath"]
    need(input_manifest.is_file() and sha(input_manifest) == native["privateInputs"]["manifestSha256"], "Private generated-input manifest hash mismatch")
    private_input_contract(read(input_manifest), native)
    qualification = {"nativeBuildVerified": True, "standaloneMusicCpuVerified": False,
                     "standaloneMusicSourceIdentityVerified": True, "freshMusicRomRenderVerified": False,
                     "originalRomUsedForBuild": False, "preparedContentUsedForBuild": False,
                     "sourceEngineSha256": sha(game), "sourceWorkerSha256": sha(worker),
                     "nativeBuildProofSha256": sha(build_proof), "nativeBuildProvenance": native}
    qualification_contract(qualification)
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
    elf_glibc = {str(path.resolve()): glibc_requirement(path) for path in (game, worker, workshop, *dependencies.values())}
    args.output.mkdir(parents=True)
    folder = args.output / ("DinoTale-Tornie-" + args.version + "-Linux-x64-Candidate")
    folder.mkdir()
    inputs = {str(args.capabilities.resolve()): sha(args.capabilities), str((distribution / "content-patch.json").resolve()): sha(distribution / "content-patch.json"), str(publish_manifest): sha(publish_manifest), str(api_proof): sha(api_proof), str(build_proof): sha(build_proof), str(input_manifest): sha(input_manifest), str(Path(__file__).resolve()): sha(Path(__file__))}
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
    capabilities.update(version=args.version, platform="Linux x86-64", exeSha256=sha(engine / "DinoTale"), engineSha256=sha(engine / "DinoTale"),
                        menuMusicWorker="MusicRender0310", menuMusicWorkerSha256=sha(engine / "MusicRender0310"),
                        workerSha256=sha(engine / "MusicRender0310"),
                        manualUserAccepted=False, releaseReady=False, originalRomRequired=True, runtimeValidation="not-validated",
                        nativeBuildVerified=True, standaloneMusicCpuVerified=False, workshopApiVerified=True, workshopBinaryReusedFrom0340=False,
                        nativeLevelEntryVerified=False, softwareGraphicsValidation="not-run-for0355",
                        physicalLinuxMintValidation="pending",
                        validationScope="Linux0355 fresh no-ROM CI build, worker source identity, strict package and new0355 Workshop API qualified. No fresh ROM audio render, gameplay or graphics session; physical Mint and grouped human tests pending.")
    capabilities.update(qualification)
    write(engine / "engine-build.json", capabilities)
    patch = dict(old_patch, version=args.version, engineSha256=sha(engine / "DinoTale"))
    write(folder / "pc/distribution/content-patch.json", patch)
    notice_map = licences(folder, dependencies, workshop, inputs)
    write(folder / "pc/linux-runtime-libraries.json", {"schema": 1, "platform": "Linux x86-64",
          "system": system, "bundled": {name: {"sha256": sha(engine / "lib" / name), "notice": notice_map[name]} for name in sorted(dependencies)},
          "minimumGraphicsApi": "Vulkan 1.2", "targetDistribution": "Linux Mint 22.3",
          "bundlePolicy": "verified SDL2 only; desktop, TLS, C++ toolchain and drivers stay system-owned",
          "glibcMaximumAllowed": ".".join(map(str, GLIBC_BASELINE)), "buildBaseline": "Ubuntu22.04 x86-64", "embeddedDotnetNativeClosureRequiresRuntimeTest": True})
    write(folder / "pc/portable-package.json", {"format": 1, "version": args.version, "platform": "Linux x86-64",
          "freshSave": True, "personalProgressIncluded": False, "testUnlocksForced": False,
          "volumePercent": 3, "originalRomRequired": True, "romIncluded": False,
          "saveFolder": "pc/profiles/content-personal", "workshopInterface": "local browser",
          "profileAssetsBootstrap": "runtime-to-profile-before-game",
          "engineSha256": sha(engine / "DinoTale"), "workshopSha256": sha(atelier), "workshopDeployment": deployment,
          "menuMusicWorkerSha256": sha(engine / "MusicRender0310"), "releaseReady": False,
          "runtimeValidation": "not-validated", "coopAndBossBattleDeferred": True,
          **qualification, "workshopApiVerified": True,
          "nativeLevelEntryVerified": False, "softwareGraphicsValidation": "not-run-for0355",
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
    template = PC / "scripts/personal-bonus-linux0355.sh"
    inputs[str(template.resolve())] = sha(template)
    optional = template.read_text(encoding="utf-8").replace("@ENGINE_SHA256@", sha(engine / "DinoTale")).replace("@WORKER_SHA256@", sha(engine / "MusicRender0310")).replace("@ROM_SHA256@", ROM_SHA).replace("@CONTENT_SHA256@", CONTENT_SHA)
    (folder / "Bonus-en-test.sh").write_text(optional, encoding="utf-8", newline="\n")
    os.chmod(folder / "Bonus-en-test.sh", 0o755)
    french, english, spoilers = guide_texts0355()
    (folder / "LIRE-MOI.txt").write_text(french, encoding="utf-8")
    (folder / "README.txt").write_text(english, encoding="utf-8")
    (folder / "SPOILERS-OPTIONNELS.txt").write_text(spoilers, encoding="utf-8")
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
              "glibcMaximumAllowed": ".".join(map(str, GLIBC_BASELINE)),
              "elfInputGlibcRequirements": {name: ".".join(map(str, version)) for name, version in elf_glibc.items()},
              "workshopBundleByteExact": sha(workshop) == sha(atelier),
              "runtimeValidation": "not-validated", "hardwareMintValidation": "pending",
              **qualification, "workshopApiVerified": True,
              "nativeLevelEntryVerified": False, "softwareGraphicsValidation": "not-run-for0355",
              "createdUtc": datetime.now(timezone.utc).isoformat()}
    write(args.output / "package-report.json", report)
    print(json.dumps({k: report[k] for k in ("folder", "archive", "archiveSha256", "fileCount", "runtimeValidation")}, indent=2))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--runtime", type=Path, default=PC / "build/native-linux-0355")
    p.add_argument("--workshop", type=Path, default=PC / "build/workshop-linux0355")
    p.add_argument("--capabilities", type=Path, default=PC / "build/native-linux-0355/engine-build.json")
    p.add_argument("--build-proof", type=Path, default=PC / "build/linux-mint-0355/native-build-final.json")
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--version", default="0.35.5")
    p.add_argument("--plan", action="store_true", help="Only list required inputs; no tools or writes")
    args = p.parse_args()
    need(args.version == "0.35.5", "This policy only stages0355")
    args.output = args.output.resolve()
    need(args.output.is_relative_to((ROOT / "dist").resolve()) and args.output.name == "Linux-0.35.5-Candidate-01",
         "Only a new local dist candidate directory is allowed")
    if args.plan:
        required = [args.runtime / "DinoTale", args.runtime / "MusicRender0310", args.runtime / "assets", args.build_proof,
                    args.workshop / "YoshiStory-AtelierLinux", args.workshop / "publish-manifest0355.json", args.capabilities,
                    PC / "profiles/portable-fresh-seed-20261028/saves/yoshisstory.n64.us.bin",
                    PC / "build/publication-0150/distribution/content.bps"]
        print(json.dumps({"required": {str(path): path.exists() for path in required}, "output": str(args.output),
                          "existingOutputRefused": args.output.exists(), "toolsRequired": ["ldd", "ldconfig", "patchelf", "readelf", "dpkg-query"]}, indent=2))
        return
    stage(args)


if __name__ == "__main__":
    main()