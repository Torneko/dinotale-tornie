# Building DinoTale Tornie 0.35.5-beta.1

Use a dedicated development checkout. The public tree contains no original ROM, prepared game image, complete generated game functions, generated RSP source, personal progression or private cache. The source archive accompanies both testing downloads; its dependency manifest records all pinned libraries. Compiler and SDK changes can produce different executable bytes.

## Checkout and dependencies

```sh
git clone --recurse-submodules --branch v0.35.5-beta.1 https://github.com/Torneko/dinotale-tornie.git
cd dinotale-tornie
```

GitHub’s automatic ZIP and the curated source ZIP do not populate library submodules. From a source ZIP, obtain the Git checkouts at the exact commits recorded in `.gitmodules`, `upstream.json` and the included dependency-pin manifest; initialize their nested submodules. A recursive tagged Git checkout is the simplest build route. CMake applies the retained `pc/dinotale/lib-patches` series, so use disposable library checkouts rather than an installed game folder.

Windows native builds use a Visual Studio developer environment, Clang20 and Ninja. Linux binaries were compiled on Ubuntu22.04 with Clang15, glibc2.35 and SDL2.30.3. Desktop build prerequisites include GTK3, Freetype, X11 and Vulkan development packages; pinned RT64 submodules contain DXC. Linux Mint22.3 is the gameplay target, with human testing pending. See [upstream prerequisites](pc/dinotale/BUILDING.md).

The Workshop uses .NET SDK9.0.300 and a self-contained .NET/ASP.NET9.0.5 runtime. Recipient machines do not need an SDK or .NET installation. Native graphics drivers and desktop libraries remain system dependencies.

## Generate local tools and game functions

Build the pinned N64Recomp tools first:

```sh
cmake -S pc/dinotale/lib/N64ModernRuntime/N64Recomp -B pc/build/recompiler-tools -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build pc/build/recompiler-tools --target N64RecompCLI RSPRecomp --parallel 2
```

On Windows use the Clang/Visual Studio developer shell and the resulting `.exe` tools. Point the preparation command at an original USA ROM in any supported byte order:

```sh
python3 pc/scripts/prepare-content0355.py --rom /path/to/original-USA-ROM.z64 --recompiler pc/build/recompiler-tools/N64Recomp
```

This invokes the original preparation path, verifies the BPS reconstruction and 219-file baseline, applies the verified 0.34.0 hooks and then the incremental owned 0.35.5 hooks. All final generated files must match `pc/recompile/generated-0355-sha256.json` after newline normalization. It rejects an existing output directory and preserves the selected ROM. The original `.z64/.v64/.n64` is neither copied into the public source tree nor changed.

## Native engine and standalone audio worker

Linux:

```sh
python3 pc/scripts/build-current0355.py --platform linux --rsp-recompiler pc/build/recompiler-tools/RSPRecomp --jobs 2
```

Windows, from the developer shell:

```sh
python pc/scripts/build-current0355.py --platform windows --rsp-recompiler pc/build/recompiler-tools/RSPRecomp.exe --jobs 2
```

Add `--sdl2-dir /path/to/SDL2/cmake` when required. The helper uses only the locally prepared, hash-verified content image to generate audio microcode through `prepare-audio0355.py`. It retains provenance and refuses to overwrite an unrelated existing RSP file. The CMake integration then compiles that local RSP source directly into the game and the CPU-only `MusicRender0310` worker; it does not require a second original ROM at the upstream root. `WORKSHOP_NATIVE_COOP` is an existing native ABI build definition, not a statement that cooperative play is finished.

RSPRecomp switch-case ordering can vary between toolchains. A newly generated audio source must be qualified for its own renderer identity; the release does not promise identical generated RSP text or executable bytes on every compiler. Do not reuse a prior identity receipt to certify a different source.

The runtime output is `pc/build/native-special`. A rebuilt executable needs a `content-patch.json` whose `engineSha256` matches its actual SHA256 before using the local Workshop importer. Keep `sourceSha256`, `targetSha256`, sizes and BPS hash unchanged. Review this binding in a dedicated development checkout; a source build must not overwrite another player’s package or progression.

## Workshop and portable launcher

```sh
dotnet publish pc/workshop/Workshop/Workshop.csproj -c Release -r win-x64 --self-contained true -p:RuntimeFrameworkVersion=9.0.5 -p:Version=0.35.5 -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -o pc/build/workshop-0.21.0
dotnet publish pc/workshop/WorkshopLinux/WorkshopLinux.csproj -c Release -r linux-x64 --self-contained true -p:RuntimeFrameworkVersion=9.0.5 -p:Version=0.35.5 -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -o pc/build/workshop-linux
```

`workshop-0.21.0` is the retained Windows lookup path and does not indicate an older editor. The portable Windows bootstrap source remains `pc/workshop/PortableLauncherNative0340`, with the original build recipe in the [0.34.0 build notes](docs/BUILDING-0.34.0-beta.1.md). The bootstrap was reused unchanged; engine and Workshop components were updated. Linux entry points are `Jouer.sh` and `Atelier.sh`.

Packaging scripts under `pc/scripts` describe the reviewed inputs and checks; release exports additionally require the matching redistributable libraries, licenses, importer bindings and a factory-empty seed. Preserve the content delta, notices, assets, audio worker and manifests. Never include the original/prepared game image, personal profile, test-created unlocks, local caches or a build-transfer key.

## Verification and release provenance

The release verification and source manifests distinguish final tagged sources from the exact native compile commit. Documentation and website changes can follow a binary build; canonical production source inputs must remain identical. The source archive exports raw Git blobs, not history or private encrypted build-input refs. Windows line-ending conversion is not a valid substitute for raw Git-object hashes.

Native CPU checks and real headless Workshop API checks do not certify complete gameplay. No fresh Linux ROM-backed music render is claimed by the ROM-free remote workflow: its worker source identity is bound to previously qualified Windows/Linux renders. New LinuxMint gameplay, rendering, and grouped mode tests remain pending. Co-op and Boss Battle remain unfinished testing features.

```sh
python scripts/check-site.py
```

This validates the bilingual website, its explicit spoiler gate and download links. The static `website` folder is deployed through GitHub Pages. No ROM is needed for guide CI. Historical release instructions remain in the tagged earlier versions and [archived 0.34.0 notes](docs/BUILDING-0.34.0-beta.1.md).
