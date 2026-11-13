"""Verify a pristine Linux0355 candidate without starting a game or GPU.

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
from types import SimpleNamespace
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch

sys.dont_write_bytecode = True
_module = Path(__file__).with_name("package-linux0355.py")
_spec = importlib.util.spec_from_file_location("linux_package0355", _module)
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
    package.qualification_contract(meta)
    deployment = meta.get("workshopDeployment", {})
    need(deployment.get("exeSha256") == meta.get("workshopSha256") and deployment.get("runtimeIdentifier") == "linux-x64" and
         all(deployment.get(key) is True for key in ("selfContained", "publishSingleFile", "includeNativeLibrariesForSelfExtract")),
         "Self-contained Workshop publish evidence missing")
    package.deployment_contract(deployment, meta.get("workshopSha256"))
    engine = folder / "pc/build/native-special"
    atelier = folder / "pc/build/workshop-linux/YoshiStory-AtelierLinux"
    game, worker = engine / "DinoTale", engine / "MusicRender0310"
    capabilities = read(engine / "engine-build.json")
    package.feature_contract(capabilities)
    package.qualification_contract(capabilities)
    need(capabilities.get("nativeBuildProvenance") == meta.get("nativeBuildProvenance") and
         capabilities.get("nativeBuildProofSha256") == meta.get("nativeBuildProofSha256"), "Build provenance differs between package and engine")
    need(capabilities.get("customLevelVersion") == 6 and capabilities.get("soloYoshiColors") == 18 and
         capabilities.get("customYoshiColors") == 18 and capabilities.get("editorMetallicPlacements") is True and
         capabilities.get("storyHybrid") is True and capabilities.get("storyHard") is True and
         capabilities.get("storyHardPublicEntitlementCurrentlyAvailable") is True and
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
    need(set(libraries) == package.PRIVATE_LIBS and libmeta.get("glibcMaximumAllowed") == "2.35", "Only controlled SDL and the Ubuntu22.04/glibc2.35 baseline are allowed")
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
    bonus = (folder / "Bonus-en-test.sh").read_text(encoding="utf-8")
    need(bonus.startswith("#!/bin/sh\n") and "\r" not in bonus and "@ENGINE_SHA256@" not in bonus and
         sha(game) in bonus and sha(worker) in bonus and "@WORKER_SHA256@" not in bonus and package.ROM_SHA in bonus and package.CONTENT_SHA in bonus and 'flock -n 9' in bonus and
         '--workshop-bonus-finales0353 1 24' in bonus and 'bonus0353-qa-linux' in bonus and
         'DinoTale Bonus0353 complete route preview only' in bonus and
         not (folder / "pc/profiles/bonus0353-qa-linux").exists(), "Bonus preview launcher is not pristine/bound")
    need(meta.get("version") == "0.35.5" and capabilities.get("bossYoshiColors") == 18 and
         capabilities.get("bossChallengeSchema", capabilities.get("bossChallengeLedgerSchema")) == 3, "Latest Boss18 capability metadata missing")
    french, english, spoilers = package.guide_texts0355()
    for name, expected in (("LIRE-MOI.txt", french), ("README.txt", english), ("SPOILERS-OPTIONNELS.txt", spoilers)):
        need((folder / name).read_text(encoding="utf-8") == expected, "Offline guide differs: " + name)
    report = {"passed": True, "platform": "Linux x86-64", "package": str(folder),
              "fileCount": len(inventory), "allFileHashesVerified": True, "strictWhitelist": True,
              "freshSaveVerified": True, "romIncluded": False, "noPersonalOrQaLedgers": True,
              "featureParity0355MetadataVerified": True, "volumePercent": 3,
              "noRomCiQualificationVerified": True, "freshMusicRomRenderVerified": False,
              "workerQualification": meta["nativeBuildProvenance"]["workerQualification"],
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
            maximum = max(maximum, package.glibc_requirement(path))
            checks[path.relative_to(folder).as_posix()] = sorted(deps)
        need(maximum <= package.GLIBC_BASELINE, "ELF inputs require newer glibc than Ubuntu22.04/2.35")
        for path, expected in ((game, "$ORIGIN/lib"), (worker, "$ORIGIN/lib")):
            need(package.command(["patchelf", "--print-rpath", path]).strip() == expected, "Incorrect staged RPATH")
        report.update(elfDependencyClosureVerified=True, glibcMaximumRequired=".".join(map(str, maximum)), dependencyChecks=checks)
        if probe_worker:
            result = subprocess.run([str(worker), "--identity"], cwd=engine, env=env, text=True,
                                    capture_output=True, timeout=10)
            identity = result.stdout.strip()
            need(result.returncode == 0 and identity == meta["nativeBuildProvenance"]["workerQualification"]["rendererIdentity"], "CPU music worker identity differs from qualified generated sources")
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
                     "pc/profiles/content-personal/pc-metal-yoshis.json", "pc/profiles/content-personal/pc-elemental-yoshis.json", "pc/profiles/content-personal/pc-heart-wallet.json",
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

    def test_ubuntu2204_glibc_baseline_is_enforced(self):
        self.assertEqual(package.GLIBC_BASELINE, (2, 35))
        with patch.object(package, "command", return_value="GLIBC_2.2.5 GLIBC_2.35 GLIBCXX_3.4.30"):
            self.assertEqual(package.glibc_requirement(Path("synthetic-elf")), (2, 35))
        for future in ("GLIBC_2.36", "GLIBC_2.39", "GLIBC_3.0"):
            with patch.object(package, "command", return_value=future):
                with self.assertRaises(ValueError): package.glibc_requirement(Path("synthetic-elf"))
        with tempfile.TemporaryDirectory() as temp:
            folder = self.fixture(temp)
            path = folder / "pc/linux-runtime-libraries.json"
            value = read(path); value["glibcMaximumAllowed"] = "2.39"
            package.write(path, value); self.inventory(folder)
            with self.assertRaises(ValueError): validate(folder, static_only=True)

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

    @staticmethod
    def native_fixture(digest):
        return {"passed": True, "version": "0.35.5", "engineSha256": digest, "workerSha256": digest,
            "freshNativeBuild": True, "inputsUnchanged": True, "originalRomUsed": False, "preparedContentUsed": False,
            "engineBuildMetadataSha256": "0" * 64,
            "privateInputs": {"manifestPath": "pc/build/synthetic-inputs.json", "generatedCount": 219,
                "bundlePlaintextSha256": "0" * 64, "manifestSha256": "0" * 64,
                "generatedManifestSha256": "0" * 64, "rspSha256": "0" * 64},
            "contentCompiled": {"expectedSha256": package.CONTENT_SHA, "fnv64": "5E83770716DC3E31", "placeholderOnly": True, "realContentUploaded": False},
            "workerQualification": {"identityExecuted": True, "actualRenderExecuted": False,
                "sourceIdentityVerified": True, "rendererIdentity": "0" * 64,
                "priorAudioReference": {"windows0355": "synthetic prior Windows proof", "linux0354": "synthetic prior Linux proof",
                    "rendererIdentity": "0" * 64, "pcmSha256": "950824e78db6b852ddf7ebb8903d19ea7e433a060b130fa85852d4363f3c49f1",
                    "wavSha256": "5f1b5e3b322eda0eb35e5d3c948e92e56f29a922f9e0f7af394665173c5565db"}}}

    def test_no_rom_ci_build_proof_rejects_unqualified_or_misleading_claims(self):
        digest = "1" * 64
        proof = self.native_fixture(digest)
        package.native_contract(proof, digest, digest, "0" * 64)
        for section,key,value in [(None,"freshNativeBuild",False),(None,"originalRomUsed",True),
                (None,"preparedContentUsed",True),(None,"workerSha256","2"*64),
                (None,"inputsUnchanged",False),("privateInputs","generatedCount",218),
                ("privateInputs","manifestPath","../outside.json"),("privateInputs","rspSha256","bad"),
                ("contentCompiled","realContentUploaded",True),("contentCompiled","placeholderOnly",False),("contentCompiled","fnv64","0000000000000000"),
                ("workerQualification","actualRenderExecuted",True),("workerQualification","identityExecuted",False),
                ("workerQualification","sourceIdentityVerified",False),("workerQualification","priorAudioReference",{})]:
            changed = json.loads(json.dumps(proof))
            (changed[section] if section else changed)[key] = value
            with self.assertRaises(ValueError,msg=str((section,key))): package.native_contract(changed,digest,digest,"0"*64)
        with self.assertRaises(ValueError): package.native_contract(proof,digest,digest,"3"*64)
        with tempfile.TemporaryDirectory() as temp:
            folder = self.fixture(temp)
            for name in ("pc/portable-package.json", "pc/build/native-special/engine-build.json"):
                path = folder/name
                saved = read(path)
                for key,value in (("standaloneMusicCpuVerified",True),("freshMusicRomRenderVerified",True),("originalRomUsedForBuild",True)):
                    changed = dict(saved); changed[key] = value
                    package.write(path,changed); self.inventory(folder)
                    with self.assertRaises(ValueError,msg=key): validate(folder,static_only=True)
                package.write(path,saved); self.inventory(folder)

    def test_generated_only_private_manifest_rejects_rom_content_and_save_entries(self):
        normalized = {"funcs_" + str(index) + ".c": "0" * 64 for index in range(216)}
        normalized.update({name:"0"*64 for name in ("lookup.cpp","funcs.h","recomp_overlays.inl")})
        generated_sha = package.hashlib.sha256(json.dumps(normalized,sort_keys=True,separators=(",", ":")).encode()).hexdigest()
        manifest = {"schema":1,"version":"0.35.5","originalRomIncluded":False,"preparedContentIncluded":False,"saveIncluded":False,
            "generatedCount":219,"contentSha256":package.CONTENT_SHA,"contentFnv64":"5E83770716DC3E31",
            "generatedNormalizedSha256":normalized,"generatedManifestSha256":generated_sha,"rspSha256":"0"*64,
            "files":{"RecompiledFuncs/"+name:"0"*64 for name in normalized}}
        manifest["files"]["rsp/aspMain.cpp"]="0"*64
        proof=self.native_fixture("1"*64)
        proof["privateInputs"]["generatedManifestSha256"]=generated_sha
        package.private_input_contract(manifest,proof)
        for key in ("originalRomIncluded","preparedContentIncluded","saveIncluded"):
            changed=json.loads(json.dumps(manifest));changed[key]=True
            with self.assertRaises(ValueError,msg=key):package.private_input_contract(changed,proof)
        for name in ("rom/baserom.z64","runtime/workshop-content.bin","profiles/saves/personal.bin","RecompiledFuncs/extra.c"):
            changed=json.loads(json.dumps(manifest));changed["files"][name]="0"*64
            with self.assertRaises(ValueError,msg=name):package.private_input_contract(changed,proof)
        changed=json.loads(json.dumps(manifest));changed["generatedNormalizedSha256"]["funcs_0.c"]="1"*64
        with self.assertRaises(ValueError):package.private_input_contract(changed,proof)

    def test_prepared_or_placeholder_content_is_not_a_packaging_input(self):
        with tempfile.TemporaryDirectory() as temp:
            runtime = Path(temp)/"native"
            runtime.mkdir()
            prepared = runtime/"workshop-content.bin"
            prepared.write_bytes(b"synthetic placeholder must never be read or copied")
            output = Path(temp)/"new-output"
            before = sha(prepared)
            with patch.object(package.sys,"platform","linux"), patch.object(package,"elf64",side_effect=AssertionError("Content guard must run before reading ELF or other inputs")):
                with self.assertRaises(ValueError):
                    package.stage(SimpleNamespace(output=output,runtime=runtime,workshop=Path(temp)/"workshop"))
            self.assertFalse(output.exists())
            self.assertEqual(sha(prepared),before)

    def test_existing_candidate_directory_is_never_mutated(self):
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp)/"existing-candidate"
            output.mkdir()
            sentinel = output/"old-export.tar.gz"
            sentinel.write_bytes(b"preexisting export must remain exact")
            before = sha(sentinel)
            with patch.object(package.sys,"platform","linux"):
                with self.assertRaises(ValueError): package.stage(SimpleNamespace(output=output))
            self.assertEqual(sha(sentinel),before)
            self.assertEqual([item.name for item in output.iterdir()],[sentinel.name])

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
        native = self.native_fixture(game_hash)
        qualification = {"nativeBuildVerified": True, "standaloneMusicCpuVerified": False,
            "standaloneMusicSourceIdentityVerified": True, "freshMusicRomRenderVerified": False,
            "originalRomUsedForBuild": False, "preparedContentUsedForBuild": False,
            "sourceEngineSha256": game_hash, "sourceWorkerSha256": game_hash,
            "nativeBuildProofSha256": "0" * 64, "nativeBuildProvenance": native}
        obj("pc/portable-package.json", {**qualification, "version": "0.35.5", "freshSave": True, "personalProgressIncluded": False, "testUnlocksForced": False,
            "romIncluded": False, "originalRomRequired": True, "volumePercent": 3, "releaseReady": False,
            "profileAssetsBootstrap": "runtime-to-profile-before-game",
            "engineSha256": game_hash, "menuMusicWorkerSha256": game_hash, "workshopSha256": game_hash,
            "workshopDeployment": {"exeSha256": game_hash, "runtimeIdentifier": "linux-x64", "selfContained": True,
                "publishSingleFile": True, "includeNativeLibrariesForSelfExtract": True, "version": "0.35.5",
                "soloYoshiColors": 18, "editorSpecialEggCount": 12, "editorElementalPlacements": True, "unknownYoshiMenuIdentitiesHidden": True,
                "nativeLinuxExecutionVerified": True, "workshopApiVerified": True, "apiProof": "pc/build/synthetic-proof.json", "apiProofSha256": "0" * 64}})
        obj("pc/build/native-special/engine-build.json", {**qualification, "customLevelVersion": 6, "soloYoshiColors": 18, "customYoshiColors": 18,
            "editorMetallicPlacements": True, "storyHybrid": True, "storyHard": True, "storyHardPublicEntitlementCurrentlyAvailable": True, "bossYoshiColors": 18, "bossChallengeSchema": 3,
            "soloElementalYoshis": True, "editorElementalPlacements": True, "editorSpecialEggCount": 12,
            "elementalOwnershipJournalSchema": 2, "unknownYoshiMenuIdentitiesHidden": True, "unknownYoshiIconsNotSelectable": True,
            "titleArtworkDesaturatesUndiscoveredHybrids": True, "storyHardPublicGate": "all16owned-and-genuineBoss16",
            "storyHardElementalPrerequisite": False, "bonusPublicEnabled": False,
            "menuMusicWorker": "MusicRender0310", "contentSha256": package.CONTENT_SHA, "releaseReady": False,
            "originalRomRequired": True, "manualUserAccepted": False, "validationScope": "Linux synthetic fixture; no runtime qualification",
            "exeSha256": game_hash, "menuMusicWorkerSha256": game_hash})
        raw("pc/distribution/content.bps", b"BPS1fixture")
        obj("pc/distribution/content-patch.json", {"format": 1, "sourceSize": 16_777_216, "targetSize": 16_777_216,
            "sourceSha256": package.ROM_SHA, "targetSha256": package.CONTENT_SHA, "patchSha256": sha(folder / "pc/distribution/content.bps"),
            "engineSha256": game_hash})
        raw("pc/build/native-special/lib/libSDL2-2.0.so.0", elf)
        raw("Licences/SDL2.txt", b"Synthetic test notice")
        obj("pc/linux-runtime-libraries.json", {"glibcMaximumAllowed": "2.35", "bundled": {"libSDL2-2.0.so.0": {"sha256": game_hash, "notice": "Licences/SDL2.txt"}}})
        template = (package.PC / "scripts/personal-bonus-linux0355.sh").read_text(encoding="utf-8")
        raw("Bonus-en-test.sh", template.replace("@ENGINE_SHA256@", game_hash).replace("@WORKER_SHA256@", game_hash).replace("@ROM_SHA256@", package.ROM_SHA).replace("@CONTENT_SHA256@", package.CONTENT_SHA).encode())
        french, english, spoilers = package.guide_texts0355()
        for name, text in (("LIRE-MOI.txt", french), ("README.txt", english), ("SPOILERS-OPTIONNELS.txt", spoilers)): raw(name, text.encode("utf-8"))
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


    def test_original16_hard_gate_and_0355_deployment_are_required(self):
        with tempfile.TemporaryDirectory() as temp:
            folder=self.fixture(temp)
            path=folder/"pc/build/native-special/engine-build.json"
            good=read(path)
            for key,value in (("soloYoshiColors",16),("bossChallengeSchema",2),("storyHardPublicGate","all18owned-and-genuineBoss18"),("storyHardElementalPrerequisite",True),("unknownYoshiMenuIdentitiesHidden",False),("bonusPublicEnabled",True)):
                changed=dict(good);changed[key]=value
                with self.assertRaises(ValueError,msg=key): package.feature_contract(changed)
            deployment=read(folder/"pc/portable-package.json")["workshopDeployment"]
            for key,value in (("version","0.34.0"),("nativeLinuxExecutionVerified",False),("workshopApiVerified",False),("apiProof","../escape"),("apiProofSha256","bad")):
                changed=dict(deployment);changed[key]=value
                with self.assertRaises(ValueError,msg=key): package.deployment_contract(changed,deployment["exeSha256"])

    def test_guides_default_to_no_discovery_spoilers(self):
        french,english,spoilers=package.guide_texts0355()
        for text in (french,english):
            self.assertIn("SPOILERS-OPTIONNELS.txt",text)
            self.assertNotRegex(text,r"\b(Feu|Eau|Fire|Water|2-2|3-4)\b")
        self.assertIn("ATTENTION SPOILERS",spoilers)
        self.assertIn("2-2",spoilers)
        self.assertIn("3-4",spoilers)

    def test_actual_elemental_awk_parser_uses_native_writer_subset(self):
        template=(package.PC/"scripts/personal-bonus-linux0355.sh").read_text(encoding="utf-8")
        program=template.split("    LC_ALL=C awk '\n",1)[1].split("\n    ' \"$file\"",1)[0]
        binary=shutil.which("awk")
        if os.name=="nt": binary=str(Path(r"C:\Program Files\Git\usr\bin\awk.exe"))
        need(binary and Path(binary).is_file(),"Actual awk parser test requires local awk; WSL is not used on Windows")
        good=[b'{"schema":1,"foundMask":0,"storyAliveMask":0}',b'{ "storyAliveMask":1, "schema":1, "foundMask":1 }\r\n',b'{"schema":2,"foundMask":3,"storyAliveMask":1,"hybridAliveMask":2}',b'\xef\xbb\xbf'+b'{"schema":2,"foundMask":0,"storyAliveMask":0,"hybridAliveMask":0}']
        bad=[b'{"schema":1,"schema":1,"foundMask":0,"storyAliveMask":0}',b'{"schema":1,"foundMask":0,"storyAliveMask":0,"x":1}',b'{"schema":2,"foundMask":1,"storyAliveMask":0,"hybridAliveMask":2}',b'{"schema":2,"foundMask":4,"storyAliveMask":0,"hybridAliveMask":0}',b'{"schema":1,"foundMask":1.0,"storyAliveMask":0}',b'{"schema":1,"foundMask":true,"storyAliveMask":0}',b'{"schema":1,"foundMask":-0,"storyAliveMask":0}',b'{"schema":1,"foundMask":0,"storyAliveMask":0,"hybridAliveMask":0}',b'{"schema":1,"foundMask":18446744073709551616,"storyAliveMask":0}',b'{"schema":2,"foundMask":0,"storyAliveMask":0}',b'{"schema":1,"f oundMask":0,"storyAliveMask":0}',b'\xff'+good[0],good[0].decode().encode("utf-16"),b'{"schema":1,"foundMask":0,"storyAliveMask":0}\x00']
        env=dict(os.environ,LC_ALL="C")
        for raw,want in [(b,True)for b in good]+[(b,False)for b in bad]:
            result=subprocess.run([binary,program],input=raw,capture_output=True,env=env,timeout=5)
            self.assertEqual(result.returncode==0,want,repr(raw)+str(result.stderr))
        self.assertIn('"$(wc -c < "$file")" -le 4096',template)
        self.assertLess(template.index('validate_elemental "$elemental"'),template.index('mkdir -p "$preview"'))


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
                  "scope": "actual whitelist, no-ROM CI provenance/audio-claim guards, existing-output preservation and safe tar extraction; no runtime or GPU",
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