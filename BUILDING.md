# Building DinoTale Tornie

The public tree contains no ROM, generated game functions, save data or private build caches. Supply your own original USA ROM. A source rebuild may produce different executable bytes because compiler/SDK versions vary.

## Checkout

```sh
git clone --recurse-submodules https://github.com/Torneko/dinotale-tornie.git
cd dinotale-tornie
```

Pinned libraries: see `upstream.json`. CMake applies the included `pc/dinotale/lib-patches` to these submodules. Use a clean dedicated checkout for builds; do not run historical private preparation scripts over this source.

## Workshop and importer (Windows)

Install .NET SDK 9.0, then:

```sh
dotnet publish pc/workshop/Workshop/Workshop.csproj -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -o pc/build/workshop-0.15.0
```

No ROM is needed to compile the UI. Opening the editor requires locally imported game data and a packaged engine. The release includes the standalone .NET runtime.

## Generate native game sources locally

The engine prerequisites (Visual Studio C++ SDK, Clang 20, CMake, Ninja, Vulkan SDK; corresponding native dependencies on Linux) are described in [the retained upstream build guide](pc/dinotale/BUILDING.md). Windows is the tested platform; Linux gameplay remains unvalidated.

Build the pinned N64Recomp from `pc/dinotale/lib/N64ModernRuntime/N64Recomp` using CMake, then:

```sh
python pc/scripts/prepare-content.py --rom /path/to/your-original-USA-ROM.z64 --recompiler /path/to/N64Recomp
```

On Windows use `N64Recomp.exe`. This creates `pc/build/special-port` only if that directory does not exist. It normalizes the ROM byte order, verifies SHA256, applies the distributed BPS delta with CRC checks, runs the recompiler, restores native link-register writes, adds opt-in diagnostics, and verifies all 219 normalized generated files against the release manifest. An unexpected recompiler output fails before compiling the engine. The source ROM is never modified.

## Native engine

Run from a configured Clang 20 / Visual Studio developer shell on Windows (use `clang-cl`); use Clang on Linux. Set `PC_ROOT` to the absolute `pc` path in this checkout:

```sh
cmake -S pc/dinotale -B pc/build/native-special -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DWORKSHOP_DIR=ABSOLUTE_PC_ROOT -DDINO_RECOMP_DIR=ABSOLUTE_PC_ROOT/build/special-port/RecompiledFuncs -DWORKSHOP_CONTENT_ROM=ABSOLUTE_PC_ROOT/build/special-port/content.z64 -DWORKSHOP_RUNTIME_OUTPUT=ABSOLUTE_PC_ROOT/build/native-special -DWORKSHOP_SPECIAL_CONTENT=ON -DWORKSHOP_HYBRID_FINDS=ON
cmake --build pc/build/native-special --target DinoTale WorkshopDataChecks --parallel 4
```

Replace each `ABSOLUTE_PC_ROOT` before running. On Linux replace both compiler values with `clang` / `clang++`; this is an experimental build, not a validated Linux delivery. Runtime DLLs/assets and app-local VC++ runtimes are needed when packaging Windows. Keep the upstream licenses. Never commit `pc/build`, generated C or game images.

## Guide

```sh
python scripts/check-site.py
python -m http.server 8000 --directory website
```

Open `http://localhost:8000/`. `index.html` is French; `index-en.html` is English. No Node build or external fonts/analytics are needed. Pages deploys from `main` through `.github/workflows/website.yml`. The CI checks the static guide without uploading a ROM.
