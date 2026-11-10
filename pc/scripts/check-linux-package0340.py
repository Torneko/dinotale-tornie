"""Verify a pristine Linux0340 candidate without starting a game or GPU.

The optional worker probe is CPU-only. Native gameplay, desktop/browser and
hardware validation belong to separate Root-owned tests. --self-test exercises
real whitelist/archive guards with temporary synthetic files only.
"""
from __future__ import annotations
import argparse
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest

sys.dont_write_bytecode = True
_module = Path(__file__).with_name("package-linux0340.py")
_spec = importlib.util.spec_from_file_location("linux_package0340", _module)
package = importlib.util.module_from_spec(_spec)
sys.modules[_spec.name] = package
_spec.loader.exec_module(package)
need, read, sha = package.need, package.read, package.sha


def members(archive):
    entries = archive.getmembers()
    need(entries and len(entries) <= 5000, "Invalid archive member count")
    root = entries[0].name.split("/", 1)[0]
    package.relative_name(root)
    seen, total = set(), 0
    for item in entries:
        name = item.name.rstrip("/")
        package.relative_name(name)
        need(name not in seen, "Duplicate archive member: " + name)
        seen.add(name)
        need(item.isfile() or item.isdir(), "Links/devices are forbidden in archive: " + name)
        need(name == root or name.startswith(root + "/"), "Multiple archive roots")
        need(item.size >= 0, "Invalid archive size")
        total += item.size
        need(total <= 1_073_741_824, "Archive expands beyond 1 GiB")
        if item.isfile():
            relative = name[len(root) + 1:]
            need(package.allowed_file(relative), "Unapproved archived file: " + relative)
    return entries, root


def unpack(archive_path, destination):
    need(not destination.exists(), "Extraction destinations are never overwritten")
    with tarfile.open(archive_path, "r:gz") as archive:
        entries, root = members(archive)
        destination.mkdir(parents=True)
        for item in entries:
            target = destination.joinpath(*item.name.rstrip("/").split("/"))
            if item.isdir():
                target.mkdir(parents=True, exist_ok=True)
                os.chmod(target, item.mode & 0o777)
            else:
                target.parent.mkdir(parents=True, exist_ok=True)
                stream = archive.extractfile(item)
                need(stream is not None, "Missing archive stream")
                with target.open("xb") as output:
                    shutil.copyfileobj(stream, output)
                os.chmod(target, item.mode & 0o777)
    return destination / root


def validate(folder, static_only=False, probe_worker=False):
    folder = folder.resolve()
    inventory = read(folder / "contenu-sha256.json")
    need(isinstance(inventory, dict) and inventory and len(inventory) <= 5000, "Invalid inventory")
    actual = {}
    for path in sorted(folder.rglob("*")):
        need(not path.is_symlink(), "Package contains a symlink")
        if not path.is_file():
            continue
        name = path.relative_to(folder).as_posix()
        package.inspect_file(path, name)
        if name != "contenu-sha256.json":
            actual[name] = sha(path)
    need(actual == inventory, "Inventory does not exactly match every package file/hash")
    for name in inventory:
        package.relative_name(name)
    profile = folder / "pc/profiles/content-personal"
    seed = profile / "saves/yoshisstory.n64.us.bin"
    need(seed.stat().st_size == 2048 and sha(seed) == package.FRESH_SHA, "Fresh EEPROM mismatch")
    need(sha(folder / "pc/save-imports/content-default.bin") == package.FRESH_SHA, "Custom save seed mismatch")
    progress = read(profile / "menu-progress.json")
    need(progress.get("valid") is True and progress.get("sourceSaveSha256") == package.FRESH_SHA and
         all(progress.get(name) == 0 for name in ("storyMask", "trialMask", "extraYoshiMask", "trialEggs")), "Non-empty progression")
    need(read(profile / "sound.json") == {"main_volume": 3}, "Initial sound must be 3%")
    graphics = read(profile / "graphics.json")
    need(graphics.get("api_option") == "Vulkan" and graphics.get("msaa_option") == "None" and
         graphics.get("rr_option") == "Original", "Incorrect Linux graphics defaults")
    meta = read(folder / "pc/portable-package.json")
    need(meta.get("freshSave") is True and meta.get("personalProgressIncluded") is False and
         meta.get("testUnlocksForced") is False and meta.get("romIncluded") is False and
         meta.get("originalRomRequired") is True and meta.get("volumePercent") == 3 and
         meta.get("releaseReady") is False and meta.get("profileAssetsBootstrap") == "runtime-to-profile-before-game",
         "Invalid fresh/candidate bootstrap metadata")
    deployment = meta.get("workshopDeployment", {})
    need(deployment.get("exeSha256") == meta.get("workshopSha256") and deployment.get("runtimeIdentifier") == "linux-x64" and
         all(deployment.get(key) is True for key in ("selfContained", "publishSingleFile", "includeNativeLibrariesForSelfExtract")),
         "Self-contained Workshop publish evidence missing")
    engine = folder / "pc/build/native-special"
    atelier = folder / "pc/build/workshop-linux/YoshiStory-AtelierLinux"
    game, worker = engine / "DinoTale", engine / "MusicRender0310"
    capabilities = read(engine / "engine-build.json")
    need(capabilities.get("customLevelVersion") == 6 and capabilities.get("soloYoshiColors") == 16 and
         capabilities.get("customYoshiColors") == 16 and capabilities.get("editorMetallicPlacements") is True and
         capabilities.get("storyHybrid") is True and capabilities.get("storyHard") is True and
         capabilities.get("storyHardPublicEntitlementCurrentlyAvailable") is False and
         capabilities.get("menuMusicWorker") == "MusicRender0310" and
         capabilities.get("contentSha256") == package.CONTENT_SHA and capabilities.get("releaseReady") is False and
         capabilities.get("originalRomRequired") is True and capabilities.get("manualUserAccepted") is False and
         str(capabilities.get("validationScope", "")).startswith("Linux"),
         "Feature parity/gate metadata mismatch")
    for path, key in ((game, "engineSha256"), (worker, "menuMusicWorkerSha256"), (atelier, "workshopSha256")):
        package.elf64(path)
        need(sha(path) == meta.get(key), "Binary metadata mismatch: " + path.name)
    need(sha(game) == capabilities.get("exeSha256") and sha(worker) == capabilities.get("menuMusicWorkerSha256"), "Capability binary hashes mismatch")
    patch = read(folder / "pc/distribution/content-patch.json")
    need(patch.get("format") == 1 and patch.get("sourceSize") == patch.get("targetSize") == 16_777_216 and
         patch.get("sourceSha256") == package.ROM_SHA and patch.get("targetSha256") == package.CONTENT_SHA and
         patch.get("patchSha256") == sha(folder / "pc/distribution/content.bps") and
         patch.get("engineSha256") == sha(game), "ROM importer hash binding mismatch")
    need((folder / "pc/distribution/content.bps").read_bytes()[:4] == b"BPS1", "Invalid BPS signature")
    libmeta = read(folder / "pc/linux-runtime-libraries.json")
    libraries = {path.name: path for path in (engine / "lib").iterdir() if path.is_file()}
    need(set(libraries) == set(libmeta.get("bundled", {})), "Dependency manifest inventory mismatch")
    need("libSDL2-2.0.so.0" in libraries, "SDL2 is missing")
    for name, path in libraries.items():
        need(not package.system_library(name), "System graphics/glibc must not be bundled: " + name)
        item = libmeta["bundled"][name]
        need(sha(path) == item.get("sha256") and package.allowed_file(item.get("notice", "")) and
             (folder / item["notice"]).is_file(), "Dependency licence/hash mismatch: " + name)
    for name in package.EXECUTABLES:
        if os.name != "nt":
            need(os.access(folder / name, os.X_OK), "Executable permission missing: " + name)
    for name, wants_play in (("Jouer.sh", True), ("Atelier.sh", False)):
        text = (folder / name).read_text(encoding="utf-8")
        need(text.startswith("#!/bin/sh\nset -eu\n") and "\r" not in text and
             '"$base/pc/build/workshop-linux/YoshiStory-AtelierLinux" "$base/pc"' in text and
             (" --play " in text) == wants_play and '"$@"' in text, "Invalid bootstrap launcher: " + name)
    report = {"passed": True, "platform": "Linux x86-64", "package": str(folder),
              "fileCount": len(inventory), "allFileHashesVerified": True, "strictWhitelist": True,
              "freshSaveVerified": True, "romIncluded": False, "noPersonalOrQaLedgers": True,
              "featureParity0330MetadataVerified": True, "volumePercent": 3,
              "executablePermissionsVerified": os.name != "nt", "elf64HeadersVerified": True,
              "runtimeGameplayTested": False, "hardwareMintTested": False, "workerProbe": "not-requested",
              "verifierSourceSha256": sha(Path(__file__)), "policySourceSha256": sha(_module)}
    if not static_only:
        need(sys.platform.startswith("linux"), "ELF loader audit requires Linux; use --static-only otherwise")
        env = dict(os.environ)
        env["LD_LIBRARY_PATH"] = str(engine / "lib")
        checks = {}
        maximum = (0, 0)
        for path in (game, worker, atelier, *libraries.values()):
            deps = package.ldd(path, env)
            for name, source in deps.items():
                if not package.system_library(name):
                    need(name in libraries and source.resolve() == libraries[name].resolve(), "Dependency escaped bundle: " + name)
            versions = package.command(["readelf", "--version-info", path])
            versions_found = [tuple(map(int, item)) for item in re.findall(r"\bGLIBC_(\d+)\.(\d+)\b", versions)]
            if versions_found:
                maximum = max(maximum, *versions_found)
            checks[path.relative_to(folder).as_posix()] = sorted(deps)
        need(maximum <= (2, 39), "ELF inputs require newer glibc than Linux Mint22.3")
        for path, expected in ((game, "$ORIGIN/lib"), (worker, "$ORIGIN/lib")):
            need(package.command(["patchelf", "--print-rpath", path]).strip() == expected, "Incorrect staged RPATH")
        report.update(elfDependencyClosureVerified=True, glibcMaximumRequired=".".join(map(str, maximum)), dependencyChecks=checks)
        if probe_worker:
            result = subprocess.run([str(worker), "--identity"], cwd=engine, env=env, text=True,
                                    capture_output=True, timeout=10)
            identity = result.stdout.strip()
            need(result.returncode == 0 and re.fullmatch(r"[0-9a-f]{64}", identity), "CPU music worker identity failed")
            report.update(workerProbe="passed CPU identity only", workerRendererIdentity=identity)
    else:
        need(not probe_worker, "Worker probe is incompatible with --static-only")
        report["elfDependencyClosureVerified"] = False
    return report


class WhitelistTests(unittest.TestCase):
    def test_allowed_real_contract(self):
        for name in (*package.EXECUTABLES, "pc/profiles/content-personal/saves/yoshisstory.n64.us.bin",
                     "pc/distribution/content.bps", "pc/build/native-special/lib/libstdc++.so.6",
                     "pc/build/native-special/assets/workshop-menu/book-right-approved.png", "Licences/DotNET.txt"):
            self.assertTrue(package.allowed_file(name), name)

    def test_personal_qa_rom_and_caches_are_not_allowed(self):
        for name in ("work/baserom.z64", "pc/build/native-special/workshop-content.bin",
                     "pc/profiles/content-personal/yoshisstory.n64.us.z64", "pc/profiles/content-personal/pc-story-hard.json",
                     "pc/profiles/content-personal/pc-metal-yoshis.json", "pc/profiles/content-personal/pc-heart-wallet.json",
                     "pc/profiles/content-personal/pc-collection.json", "pc/profiles/content-personal/pc-achievements.json",
                     "pc/profiles/content-personal/saves/yoshisstory.n64.us.story-hard0330-preview.bin",
                     "pc/profiles/content-personal/story-hard-qa.txt", "pc/cache/menu-music0310/test.wav",
                     "pc/build/native-special/lib/libc.so.6/secret", "pc/build/native-special/assets/test.bin",
                     "pc/build/workshop-linux/Core.pdb", "pc/profiles/content-personal/assets/Exo2.ttf"):
            self.assertFalse(package.allowed_file(name), name)

    def test_paths(self):
        for name in ("../escape", "/absolute", "C:/secret", "a\\b", "a//b", "a/./b", "a/../b", "", "a\x00b"):
            with self.assertRaises(ValueError):
                package.relative_name(name)

    def test_image_magic_cannot_hide_in_allowed_file(self):
        with tempfile.TemporaryDirectory() as temp:
            file = Path(temp) / "innocent.png"
            for magic in package.ROM_MAGICS:
                file.write_bytes(magic + b"not-a-real-game")
                with self.assertRaises(ValueError):
                    package.inspect_file(file, "pc/build/native-special/assets/innocent.png")

    def test_regular_candidate_asset_allowed(self):
        with tempfile.TemporaryDirectory() as temp:
            file = Path(temp) / "a.png"
            file.write_bytes(b"\x89PNG\r\n\x1a\n")
            package.inspect_file(file, "pc/build/native-special/assets/a.png")

    def test_only_reviewed_sdl_is_bundled_desktop_toolchain_stay_system(self):
        for name in ("libc.so.6", "libm.so.6", "ld-linux-x86-64.so.2", "libvulkan.so.1", "libGLX_nvidia.so.0", "libdrm.so.2", "libstdc++.so.6",
                     "libgcc_s.so.1", "libssl.so.3", "libgtk-3.so.0", "libglib-2.0.so.0", "libpulse.so.0"):
            self.assertTrue(package.system_library(name), name)
        for name in ("libSDL2-2.0.so.0",):
            self.assertFalse(package.system_library(name), name)

    def test_archive_rejects_traversal_link_device_duplicate_roots(self):
        cases = [ [("candidate/../escape", "file")], [("candidate/a", "symlink")],
                  [("candidate/a", "hardlink")], [("candidate/a", "device")],
                  [("candidate/Jouer.sh", "file"), ("candidate/Jouer.sh", "file")],
                  [("candidate/Jouer.sh", "file"), ("other/Atelier.sh", "file")],
                  [("candidate/pc/profiles/content-personal/pc-collection.json", "file")],
                  [("candidate/pc/build/native-special/workshop-content.bin", "file")] ]
        for index, entries in enumerate(cases):
            with tempfile.TemporaryDirectory() as temp:
                path = Path(temp) / "invalid.tar.gz"
                with tarfile.open(path, "w:gz") as archive:
                    for name, kind in entries:
                        item = tarfile.TarInfo(name)
                        if kind == "symlink": item.type = tarfile.SYMTYPE; item.linkname = "/tmp/escape"
                        elif kind == "hardlink": item.type = tarfile.LNKTYPE; item.linkname = "candidate/Jouer.sh"
                        elif kind == "device": item.type = tarfile.CHRTYPE
                        archive.addfile(item)
                with tarfile.open(path) as archive:
                    with self.assertRaises(ValueError, msg=str(index)):
                        members(archive)

    def fixture(self, directory):
        folder = Path(directory) / "candidate"
        folder.mkdir()
        def raw(name, value):
            path = folder / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(value)
            if name in package.EXECUTABLES: os.chmod(path, 0o755)
        def obj(name, value): package.write(folder / name, value)
        elf = b"\x7fELF\x02\x01" + bytes(12) + b"\x3e\x00"
        for name in package.EXECUTABLES[2:]: raw(name, elf)
        for name, flag in (("Jouer.sh", " --play"), ("Atelier.sh", "")):
            raw(name, ('#!/bin/sh\nset -eu\nexec "$base/pc/build/workshop-linux/YoshiStory-AtelierLinux" "$base/pc"' + flag + ' "$@"\n').encode())
        seed = (package.PC / "profiles/portable-fresh-seed-20261028/saves/yoshisstory.n64.us.bin").read_bytes()
        raw("pc/profiles/content-personal/saves/yoshisstory.n64.us.bin", seed)
        raw("pc/save-imports/content-default.bin", seed)
        obj("pc/profiles/content-personal/menu-progress.json", {"valid": True, "sourceSaveSha256": package.FRESH_SHA, "storyMask": 0, "trialMask": 0, "extraYoshiMask": 0, "trialEggs": 0})
        obj("pc/profiles/content-personal/sound.json", {"main_volume": 3})
        obj("pc/profiles/content-personal/graphics.json", {"api_option": "Vulkan", "msaa_option": "None", "rr_option": "Original"})
        game_hash = sha(folder / package.EXECUTABLES[2])
        obj("pc/portable-package.json", {"freshSave": True, "personalProgressIncluded": False, "testUnlocksForced": False,
            "romIncluded": False, "originalRomRequired": True, "volumePercent": 3, "releaseReady": False,
            "profileAssetsBootstrap": "runtime-to-profile-before-game",
            "engineSha256": game_hash, "menuMusicWorkerSha256": game_hash, "workshopSha256": game_hash,
            "workshopDeployment": {"exeSha256": game_hash, "runtimeIdentifier": "linux-x64", "selfContained": True,
                "publishSingleFile": True, "includeNativeLibrariesForSelfExtract": True}})
        obj("pc/build/native-special/engine-build.json", {"customLevelVersion": 6, "soloYoshiColors": 16, "customYoshiColors": 16,
            "editorMetallicPlacements": True, "storyHybrid": True, "storyHard": True, "storyHardPublicEntitlementCurrentlyAvailable": False,
            "menuMusicWorker": "MusicRender0310", "contentSha256": package.CONTENT_SHA, "releaseReady": False,
            "originalRomRequired": True, "manualUserAccepted": False, "validationScope": "Linux synthetic fixture; no runtime qualification",
            "exeSha256": game_hash, "menuMusicWorkerSha256": game_hash})
        raw("pc/distribution/content.bps", b"BPS1fixture")
        obj("pc/distribution/content-patch.json", {"format": 1, "sourceSize": 16_777_216, "targetSize": 16_777_216,
            "sourceSha256": package.ROM_SHA, "targetSha256": package.CONTENT_SHA, "patchSha256": sha(folder / "pc/distribution/content.bps"),
            "engineSha256": game_hash})
        raw("pc/build/native-special/lib/libSDL2-2.0.so.0", elf)
        raw("Licences/SDL2.txt", b"Synthetic test notice")
        obj("pc/linux-runtime-libraries.json", {"bundled": {"libSDL2-2.0.so.0": {"sha256": game_hash, "notice": "Licences/SDL2.txt"}}})
        self.inventory(folder)
        return folder

    @staticmethod
    def inventory(folder):
        package.write(folder / "contenu-sha256.json", {path.relative_to(folder).as_posix(): sha(path)
            for path in sorted(folder.rglob("*")) if path.is_file() and path.name != "contenu-sha256.json"})

    def test_complete_pristine_fixture_without_runtime_claims(self):
        with tempfile.TemporaryDirectory() as temp:
            result = validate(self.fixture(temp), static_only=True)
            self.assertTrue(result["passed"])
            self.assertFalse(result["runtimeGameplayTested"])
            self.assertFalse(result["hardwareMintTested"])
            self.assertFalse(result["elfDependencyClosureVerified"])

    def test_rehashed_personal_ledger_is_still_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = self.fixture(temp)
            package.write(folder / "pc/profiles/content-personal/pc-collection.json", {"purchases": 7})
            self.inventory(folder)
            with self.assertRaises(ValueError): validate(folder, static_only=True)

    def test_rehashed_save_tampering_is_still_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = self.fixture(temp)
            path = folder / "pc/profiles/content-personal/saves/yoshisstory.n64.us.bin"
            changed = bytearray(path.read_bytes()); changed[100] ^= 1; path.write_bytes(changed)
            self.inventory(folder)
            with self.assertRaises(ValueError): validate(folder, static_only=True)

    def test_rehashed_wrong_import_binding_and_nonselfcontained_publish_rejected(self):
        for bad in ("import", "deployment"):
            with tempfile.TemporaryDirectory() as temp:
                folder = self.fixture(temp)
                if bad == "import":
                    path = folder / "pc/distribution/content-patch.json"
                    value = read(path); value["engineSha256"] = "0" * 64
                else:
                    path = folder / "pc/portable-package.json"
                    value = read(path); value["workshopDeployment"]["selfContained"] = False
                package.write(path, value); self.inventory(folder)
                with self.assertRaises(ValueError): validate(folder, static_only=True)

    def test_extract_preserves_permissions_and_refuses_existing_target(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "valid.tar.gz"
            with tarfile.open(path, "w:gz") as archive:
                root = tarfile.TarInfo("candidate"); root.type = tarfile.DIRTYPE; root.mode = 0o755; archive.addfile(root)
                launcher = tarfile.TarInfo("candidate/Jouer.sh"); launcher.mode = 0o755; archive.addfile(launcher)
            destination = Path(temp) / "extracted"
            folder = unpack(path, destination)
            self.assertTrue((folder / "Jouer.sh").is_file())
            if os.name != "nt": self.assertTrue(os.access(folder / "Jouer.sh", os.X_OK))
            with self.assertRaises(ValueError): unpack(path, destination)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("target", type=Path, nargs="?")
    p.add_argument("--extract", type=Path, help="New isolated directory when target is tar.gz")
    p.add_argument("--report", type=Path)
    p.add_argument("--static-only", action="store_true")
    p.add_argument("--worker-identity", action="store_true")
    p.add_argument("--self-test", action="store_true")
    args = p.parse_args()
    if args.self_test:
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(WhitelistTests)
        result = unittest.TextTestRunner(verbosity=2).run(suite)
        report = {"passed": result.wasSuccessful(), "testMethods": result.testsRun,
                  "failures": len(result.failures), "errors": len(result.errors),
                  "scope": "actual whitelist, image guards and safe tar extraction; no runtime or GPU",
                  "platform": sys.platform, "pythonVersion": sys.version.split()[0],
                  "verifierSourceSha256": sha(Path(__file__)), "policySourceSha256": sha(_module)}
        if args.report:
            need(not args.report.exists(), "Existing verification reports are never overwritten")
            package.write(args.report, report)
        print(json.dumps(report, indent=2))
        raise SystemExit(0 if result.wasSuccessful() else 1)
    need(args.target is not None, "A candidate folder or tar.gz is required")
    archive_sha = None
    if args.target.is_file():
        need(args.extract is not None, "Specify --extract for an archive")
        archive_sha = sha(args.target)
        folder = unpack(args.target, args.extract)
    else:
        need(args.extract is None, "--extract only applies to an archive")
        folder = args.target
    report = validate(folder, args.static_only, args.worker_identity)
    if archive_sha: report["archiveSha256"] = archive_sha
    if args.report:
        need(not args.report.exists(), "Existing verification reports are never overwritten")
        package.write(args.report, report)
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()