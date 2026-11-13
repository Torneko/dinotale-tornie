#!/usr/bin/env bash
set -euo pipefail
# Ubuntu22.04 only; remote fresh checkout, never the user's installed runtime.
test "$(. /etc/os-release; printf '%s' "$VERSION_ID")" = 22.04
sudo apt-get update -qq
sudo apt-get install -y --no-install-recommends clang-15 lld-15 cmake ninja-build git curl ca-certificates openssl \
  libgtk-3-dev libvulkan-dev libx11-dev libxrandr-dev libxcursor-dev libxi-dev libxext-dev \
  libxinerama-dev libxxf86vm-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev \
  libasound2-dev libpulse-dev libudev-dev libdbus-1-dev patchelf python3
pc="$(cd "$(dirname "$0")/../.." && pwd)"
archive="$pc/downloads/SDL2-2.30.3.tar.gz"
mkdir -p "$pc/downloads" "$pc/tools/linux-sources" "$pc/build"
curl --fail --location --silent --show-error https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-2.30.3.tar.gz -o "$archive"
printf '%s  %s\n' 820440072f8f5b50188c1dae104f2ad25984de268785be40c41a099a510f0aec "$archive" | sha256sum --check --strict
tar -xzf "$archive" -C "$pc/tools/linux-sources"
cmake -S "$pc/tools/linux-sources/SDL2-2.30.3" -B "$pc/build/linux-sdl" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$pc/tools/sdl2-linux" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF -DSDL_TESTS=OFF
cmake --build "$pc/build/linux-sdl" --parallel 2
cmake --install "$pc/build/linux-sdl"
chmod +x "$pc/dinotale/lib/rt64/src/contrib/dxc/bin/x64/dxc-linux"
