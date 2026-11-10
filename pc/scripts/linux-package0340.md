# Linux 0.34.0 local candidate

These scripts stage and check the new Linux package only. They do not build the
shared engine, launch a game, publish anything, or touch a working personal save.

Use **WSL Ubuntu-22.04**, never the default distribution. Root owns the native
build and all GPU tests. The required native runtime is `pc/build/native-linux-0340`.
The Workshop publish is `pc/build/workshop-linux0340` and must contain the ELF
`YoshiStory-AtelierLinux` plus `publish-manifest0340.json`, binding its SHA-256 to
`linux-x64`, `selfContained`, `publishSingleFile` and
`includeNativeLibrariesForSelfExtract` all true.

Preparation, once the new ELF files exist:

```sh
python3 -B /mnt/d/Yoshi-Story/pc/scripts/package-linux0340.py \
  --output /mnt/d/Yoshi-Story/dist/Linux-0.34.0-Candidate-01
```

The output path must not exist. Every new attempt gets a new path. The script
copies a strict whitelist, patches only staged game/worker RPATHs, preserves the
.NET single-file ELF byte for byte, writes new Linux manifests, and creates a
`.tar.gz` with executable permissions. Original files are hashed before/after.
The existing Windows BPS and fresh EEPROM seed are read only.

The package contains no ROM, generated `workshop-content.bin`, personal journal,
QA marker, music PCM, log or crash dump. A pristine 2048-byte EEPROM is included;
all currencies, records, Story/Trial progress and extra Yoshis start empty. Sound
starts at 3%. `Jouer.sh` invokes the Workshop with `--play`; `Atelier.sh` opens the
editor. First use prepares content locally through the authenticated import UI.
The apphost must copy the runtime assets into the active profile before every
native launch so Main keeps the profile as its CWD. Those generated profile assets
are intentionally absent from the pristine archive; the engine owns the single
distributed assets copy. This bootstrap behavior needs the apphost integration test.

Library policy approved for Linux Mint22.3: bundle the controlled SDL2 build only.
The coherent desktop, C++ toolchain, OpenSSL3, glibc and graphics drivers stay
system-owned. The exact required SONAME list is recorded after `ldd` inspection.
A self-contained .NET publish removes the recipient's .NET installation step,
not its ordinary Linux desktop dependencies. The launchers expose only this
private SDL directory in `LD_LIBRARY_PATH`.

Check a newly extracted, isolated copy before any interactive use:

```sh
python3 -B /mnt/d/Yoshi-Story/pc/scripts/check-linux-package0340.py \
  /mnt/d/Yoshi-Story/dist/Linux-0.34.0-Candidate-01/DinoTale-Tornie-0.34.0-Linux-x64-Candidate.tar.gz \
  --extract '/mnt/d/Yoshi-Story/pc/build/linux-0340/Paquet déplacé é-01' \
  --worker-identity \
  --report /mnt/d/Yoshi-Story/pc/build/linux-0340/package-check-01.json
```

Extraction and report paths must be new. Checks cover exact inventory, image
magic, fresh EEPROM, absent personal/QA state, feature metadata, Linux importer
hash binding, permissions, ELF64 architecture, all dependency resolutions, glibc
version requirements, licences and RPATHs. `--worker-identity` executes only the
CPU music helper's identity command; no ROM or GPU is needed.

`--self-test` uses temporary synthetic archives and minimal package fixtures to
exercise traversal/link/device/duplicate rejection, disguised image detection,
rehashed private-ledger/save corruption, wrong importer binding and false
self-contained claims. `--static-only` can check the archive on Windows but does
not certify the Linux dynamic loader.

The candidate records the failed WSL software graphical trials: intro stalled at
native frame11/13 with no playable Yoshi. Native build, standalone music CPU and
Workshop API evidence remain distinct. No more GPU tests are authorized in this lot.

The reports deliberately do **not** claim Mint hardware compatibility, complete
playability, natural Story Hard progression or grouped manual tests. Root's
separate bounded native tests and the later human Mint tests supply that evidence.
The scripts create local archives only; they never upload assets or modify the website or GitHub repository.
## Public experimental prerelease conversion

Root may convert the unchanged reviewed Candidate01 on Windows, without starting
WSL or rewriting any ELF:

```powershell
python -B D:\Yoshi-Story\pc\scripts\package-linux0340.py `
  --public-prerelease-from D:\Yoshi-Story\dist\Linux-0.34.0-Candidate-01\DinoTale-Tornie-0.34.0-Linux-x64-Candidate `
  --output D:\Yoshi-Story\dist\Linux-0.34.0-beta.1-01
```

The new archive is `DinoTale-Tornie-0.34.0-beta.1-Linux-x64.tar.gz`. Only README,
source notices and distribution metadata change; all three ELF files and the BPS
import manifest stay byte exact. The new package points to source tag
`v0.34.0-beta.1` and asset `DinoTale-Tornie-0.34.0-beta.1-Sources.zip`. Root supplies
and publishes that corresponding source separately. Candidate01 is preserved.

The conversion verifies every archived file and Unix executable mode using
Python on Windows. It does not repeat the Linux loader or GPU tests. Linux
loader evidence carries through the identical binary hashes; the software WSL
intro stall and pending physical Mint test remain explicitly disclosed.