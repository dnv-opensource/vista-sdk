# Vista SDK - Gmod Explorer (Emscripten/WebGL)

An interactive Gmod tree browser compiled to WebAssembly, built on ImGui + GLFW (pongasoft emscripten-glfw) + WebGL 2.
It demonstrates that the Vista SDK C++ core is fully Emscripten-compatible.

## What it shows

- All VIS versions (v3.4a through latest) in a dropdown selector
- The full Gmod tree for the selected version, rendered as a collapsible tree
- Code/name filter to search across the tree in real time
- Per-node detail panel: category, type, mappability, definition, children/parents count, product type/selection

<p align="center">
  <img src="doc/vista-sdk-gmod-explorer-wasm-webassembly.png" alt="Vista SDK Gmod Explorer WebAssembly build - interactive Gmod tree browser for ISO 19848, showing VIS 3-11a hierarchy">
</p>

## Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) activated in your shell (`emsdk activate latest && source emsdk_env.sh`)
- CMake 3.25+
- Ninja (required by the host-tools step below. It is also the only generator Emscripten's `emcmake` supports)
- Python 3.8+ (to serve the app)

## Build (Linux)

The SDK uses a host-side code generator (`blobgen`) to embed binary resources
at compile time. When cross-compiling for Emscripten, a native build of that
tool must be produced first and passed via `DNV_VISTA_SDK_BLOBGEN_HOST_PATH`.

```bash
# From the repo root

# Step 1: build native host tools (once)
cmake -S cpp -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DDNV_VISTA_SDK_BUILD_TESTS=OFF \
    -DDNV_VISTA_SDK_BUILD_SAMPLES=OFF \
    -DDNV_VISTA_SDK_BUILD_BENCHMARKS=OFF
cmake --build build --target \
    dnv-vista-sdk-blobgen \
    dnv-vista-sdk-visversionsgenerator \
    dnv-vista-sdk-iso19848versionsgenerator

# Step 2: configure and build the Emscripten target
EM_CONFIG=$HOME/.emscripten emcmake cmake -S cpp -B build-wasm -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DDNV_VISTA_SDK_BUILD_SAMPLES=ON \
    -DDNV_VISTA_SDK_BUILD_TESTS=OFF \
    -DDNV_VISTA_SDK_BUILD_BENCHMARKS=OFF \
    -DDNV_VISTA_SDK_BLOBGEN_HOST_PATH=$PWD/build/bin/dnv-vista-sdk-blobgen \
    -DDNV_VISTA_SDK_VISVERSIONSGENERATOR_HOST_PATH=$PWD/build/bin/dnv-vista-sdk-visversionsgenerator \
    -DDNV_VISTA_SDK_ISO19848VERSIONSGENERATOR_HOST_PATH=$PWD/build/bin/dnv-vista-sdk-iso19848versionsgenerator

EM_CONFIG=$HOME/.emscripten cmake --build build-wasm -j$(nproc) --target dnv-vista-sdk-sample-emscripten
```

The output lands in `build-wasm/bin/`:

```
gmod-explorer.html
gmod-explorer.js
gmod-explorer.wasm
```

## Build (Windows & PowerShell)

Use a build directory that isn't already CMake-configured for Visual Studio (Ninja is required; VS generators aren't supported by `emcmake`).

```powershell
# One-time setup
git clone https://github.com/emscripten-core/emsdk.git C:\emsdk
cd C:\emsdk
.\emsdk.ps1 install latest
.\emsdk.ps1 activate latest
# Download ninja.exe from https://github.com/ninja-build/ninja/releases/latest, put it on PATH

# Per-session activation
C:\emsdk\emsdk.ps1 activate latest
C:\emsdk\emsdk_env.ps1
$env:PATH = "C:\ninja;" + $env:PATH

# Build
cd C:\path\to\vista-sdk
cmake -S cpp -B build-host-tools -G Ninja -DCMAKE_BUILD_TYPE=Release `
    -DDNV_VISTA_SDK_BUILD_TESTS=OFF -DDNV_VISTA_SDK_BUILD_SAMPLES=OFF -DDNV_VISTA_SDK_BUILD_BENCHMARKS=OFF
cmake --build build-host-tools `
    --target dnv-vista-sdk-blobgen `
    --target dnv-vista-sdk-visversionsgenerator `
    --target dnv-vista-sdk-iso19848versionsgenerator

$env:EM_CACHE = "$HOME\.emscripten_cache"
emcmake cmake -S cpp -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Release `
    -DDNV_VISTA_SDK_BUILD_SAMPLES=ON -DDNV_VISTA_SDK_BUILD_TESTS=OFF -DDNV_VISTA_SDK_BUILD_BENCHMARKS=OFF `
    -DDNV_VISTA_SDK_BLOBGEN_HOST_PATH="$PWD\build-host-tools\bin\dnv-vista-sdk-blobgen.exe" `
    -DDNV_VISTA_SDK_VISVERSIONSGENERATOR_HOST_PATH="$PWD\build-host-tools\bin\dnv-vista-sdk-visversionsgenerator.exe" `
    -DDNV_VISTA_SDK_ISO19848VERSIONSGENERATOR_HOST_PATH="$PWD\build-host-tools\bin\dnv-vista-sdk-iso19848versionsgenerator.exe"
cmake --build build-wasm -j $env:NUMBER_OF_PROCESSORS --target dnv-vista-sdk-sample-emscripten
```
## Run

```
python cpp/samples/wasm/serve.py build-wasm/bin
# then open http://localhost:8080/gmod-explorer.html
```

## SIMD note

The SDK JSON fast paths (`__m128i`) are compiled only when `__SSE2__` is
defined. Emscripten does not define `__SSE2__` by default, so the scalar
fallback is used. To enable WASM SIMD (requires a browser supporting
WebAssembly SIMD), add `-msimd128` to both compile and link flags.
