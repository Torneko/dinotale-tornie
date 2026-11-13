# Development candidate 0.35.5

The public downloads remain at their published version. This source update prepares a separate Linux x86-64 development candidate; it does not certify gameplay or publish a new game release. Co-op and Boss Battle still require further testing.

The native candidate uses Ubuntu 22.04, Clang 15, C17/C++20 and a controlled SDL 2.30.3 build. The Linux Workshop is rebuilt from its current sources with .NET SDK 9.0.300 and a self-contained 9.0.5 runtime. Builds use at most two compiler jobs. Graphics and gameplay testing are deferred to actual Linux PCs, including the Mint 22.3 target.

## Local source reconstruction

Follow BUILDING.md to generate and verify the 219 baseline files locally from an original USA ROM, then apply the existing 0.34.0 hooks. Apply the incremental owned hooks next:

```sh
python3 pc/scripts/apply-native-hooks0355.py pc/build/special-port
```

The result must match `pc/recompile/generated-0355-sha256.json` exactly after newline normalization. The full generated functions and RSP source remain outside the public source tree. The incremental patch was replayed against the verified 0.34.0 tree and all 219 output hashes matched. The existing published source tag and build helper still describe 0.34.0.

## Remote candidate build

The manual Linux workflow accepts a reviewed source commit and an authenticated encrypted input bundle containing only the verified generated functions and preserved RSP source. Its job has read-only repository permissions. Temporary ciphertext can be supplied on a separate build-input ref; its random 256-bit key is an Actions secret, never committed. Neither the original ROM, prepared content image nor a personal profile is sent to the runner. The build uses a one-byte CMake placeholder with the actual content hash constant, then removes the placeholder before staging. The preserved RSP is compiled directly, never regenerated from that placeholder.

Only the ROM-free candidate archive and bounded verification receipts are exported. Raw compiler diagnostics remain encrypted. The candidate contains a factory-empty save, no currency or forced unlocks, and initial sound at 3 percent. A player selects an original USA ROM locally at first launch.

The current build verifies the standalone music worker's source identity against the previously qualified Windows 0.35.5 and Linux 0.35.4 renderers. No fresh ROM-backed music render, GPU session or gameplay is claimed by this ROM-free workflow. Workshop HTTP checks and persistence tests use isolated synthetic profiles, with no personal progression merged into them.
