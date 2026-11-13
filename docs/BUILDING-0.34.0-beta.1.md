# Building DinoTale Tornie 0.34.0-beta.1

The public tree contains no ROM, complete generated game C, save data or private build caches. Build tools operate on a locally selected original USA ROM. The source archive and tagged tree accompany both binary downloads. Compiler and SDK changes can produce different executable bytes.

## Checkout and dependencies

```sh
git clone --recurse-submodules --branch v0.34.0-beta.1 https://github.com/Torneko/dinotale-tornie.git
cd dinotale-tornie
```

For a source ZIP, obtain the pinned submodules listed in `upstream.json` and `.gitmodules` at the recorded commits. GitHub's automatic source ZIP does not populate submodules either. The retained libraries are patched by `pc/dinotale/lib-patches` through CMake. Use a dedicated checkout, not a personal game folder.

Native prerequisites are described in [upstream BUILDING](pc/dinotale/BUILDING.md). The packaged Windows game is the frozen solo0.33.0 engine, built with Clang20 and the VisualStudio C++ SDK. The packaged Linux engine was built with Clang15, CMake3.22, glibc2.35 and SDL2.30.3 on Ubuntu22.04.5. Linux Mint22.3 is the target; its gameplay remains unvalidated. Linux uses the host's Vulkan/graphics drivers and system desktop libraries, not bundled driver copies.

## Prepare local game functions

Build pinned N64Recomp from `pc/dinotale/lib/N64ModernRuntime/N64Recomp`, then:

```sh
python pc/scripts/prepare-content.py --rom /path/to/original-USA-ROM.z64 --recompiler /path/to/N64Recomp
```

On Windows the recompiler is `N64Recomp.exe`. This normalizes .z64/.v64/.n64, verifies the original, applies the BPS delta, runs the recompiler and the baseline fixups, verifies219 base files, then applies `pc/recompile/native-hooks-0340.patch`. All219 final files must match `generated-0340-sha256.json` after newline normalization. The original ROM is read only. Output is the new `pc/build/special-port` directory; an existing output is rejected.

The audio microcode is also generated locally from the prepared content: it is byte-identical to the original microcode, and the public CMake configuration uses that existing local content path. No extra original ROM copy is needed in the source tree.

The patch contains the later native hook changes rather than full generated game sources. `pc/scripts/apply-native-hooks0340.py` verifies the base before applying anything and verifies the final manifest. The release preparation compared this output exactly against the Windows0330/Linux0340 engine input. It did not run a game or claim a second native build.

## Native runtime and music worker

From a Clang/VisualStudio developer shell on Windows:

```sh
python pc/scripts/build-current0340.py --platform windows --jobs 2
```

Linux, with Clang15 and suitable SDL2/Vulkan development packages:

```sh
python3 pc/scripts/build-current0340.py --platform linux --jobs 2
```

Use `--sdl2-dir /path/to/SDL2/cmake` when SDL2 is not found automatically. The helper passes the release's prepared source path and native hook definitions; the co-op definition is required for existing compiled ABI hooks and does not imply that co-op or Boss Battle are finished. The `DinoTale` target also builds the separate CPU-only `MusicRender0310` executable. Both belong in a distribution.

The worker declaration was corrected for Linux Clang15. `pc/platform-variants/windows0330/menu_music_native0310.cpp` retains the pre-Linux Windows worker source for provenance. Recompiled RSP switch-case ordering also changes text-based renderer identity between toolchains; native PCM/WAV output matched Windows byte for byte. There is no promise of identical executable bytes or renderer-cache identity after a source rebuild.

## Workshop, importer and Windows launcher

Install .NET SDK9. The shipped Workshop apphosts are self-contained and require no separately installed .NET at runtime.

```sh
dotnet publish pc/workshop/Workshop/Workshop.csproj -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -o pc/build/workshop-0.21.0
dotnet publish pc/workshop/WorkshopLinux/WorkshopLinux.csproj -c Release -r linux-x64 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -o pc/build/workshop-linux
```

The directory `workshop-0.21.0` is a retained engine lookup path; it does not indicate an older editor. The Windows package's root launcher and its exact C++ source are in `pc/workshop/PortableLauncherNative0340`. In a VisualStudio C++ developer shell, use:

```sh
cmake -S pc/workshop/PortableLauncherNative0340 -B pc/build/portable-launcher0340 -A x64
cmake --build pc/build/portable-launcher0340 --config Release
```

The packaging script also records the original native compile command. Its package icon is a standard resource from the retained upstream icon; SDK/resource metadata may vary after rebuilding. The editor bootstrap verifies the executable hash and BPS manifest, imports locally and preserves an existing unrelated save. Linux entry points are `Jouer.sh` and `Atelier.sh`.

Package scripts are retained under `pc/scripts`; only reviewed explicit inputs should be used. Each rebuilt engine requires a new `content-patch.json` bound to its executable SHA256, even when `content.bps` is unchanged. Keep all dependency licenses, assets, the music worker, fresh seed and package manifests. Do not ship the original ROM, prepared content image or a personal profile.

## Verification scope and guide

The release's public verification summary is in `docs/VALIDATION-0.34.0-beta.1.json`. Native CPU checks and real Workshop APIs passed. Windows solo tests were performed on the frozen engine, but new StoryHard/editor long-play checks remain pending. Linux WSL software graphics stalled during the intro: no playable level was reached, and Mint hardware validation remains pending. Co-op and Boss Battle are incomplete and deferred.

```sh
python scripts/check-site.py
```

The static FR/EN guide is in `website`. GitHub Pages deploys `main` through `.github/workflows/website.yml`. No ROM is needed or uploaded by guide CI.
