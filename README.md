# UniversalButtonMatrix

## Build

Build instructions for both x86 host testing and the Raspberry Pi Pico target.

**x86 (host) build — quick testing**

- Requirements: `cmake`, a C++ compiler (MSVC or GCC/MinGW).
- Configure and build the host test target:

```bash
cmake -S . -B build -DHOST_BUILD=ON -DCONSOLE_DEBUG=2 -DVARIANT_128_BUTTONS_ENABLED=ON 
cmake --build build --config Debug
./build/UniversalButtonMatrix_host.exe
```

- You can also use the included VS Code tasks and launch configs:
	- See [/.vscode/tasks.json](.vscode/tasks.json#L1) and [/.vscode/launch.json](.vscode/launch.json#L1).

**Raspberry Pi Pico build (variant 128 buttons support)**

- Requirements: `cmake`, Pico SDK. Set `PICO_SDK_PATH` to your Pico SDK location.
- Configure and build the UBM-Pico firmware:

```bash
mkdir build && cd build
PICO_SDK_PATH=/path/to/pico-sdk cmake -DVARIANT_128_BUTTONS_ENABLED=1 -DCONSOLE_DEBUG=0 -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

If you prefer to pass the path as a CMake cache variable, the build now also supports:

```bash
cmake -DPICO_SDK_PATH=/path/to/pico-sdk -DVARIANT_128_BUTTONS_ENABLED=1 -DCONSOLE_DEBUG=0 -DCMAKE_BUILD_TYPE=Release ..
```

Final build, ready to upload appear as *UniversalButtonMatrix_128BTN.uf2*

**Docker (Windows) build (optional)**

This repository includes a Dockerfile at `UBM-PicoBuild.dockerfile` for Windows-based Pico builds.

Build the Docker image from the repository root:

```powershell
# PowerShell
docker build -f UBM-PicoBuild.dockerfile -t pico-build .
```

Run the container with the project mounted and build the UBM-Pico firmware:

```powershell
docker run --rm -v ${PWD}:/work -w /work pico-build -lc "rm -rf build && cmake -S . -B build -DHOST_BUILD=OFF -DPICO_SDK_PATH=/opt/pico-sdk -DVARIANT_128_BUTTONS_ENABLED=1 -DCONSOLE_DEBUG=0 -DCMAKE_BUILD_TYPE=Release && cmake --build build"
```

Notes:
- Update `UBM-PicoBuild.dockerfile` if you need a different Pico SDK version or toolchain.
- On Windows, ensure Docker has access to your drive when mounting volumes.

**TODO: (future development)**
- Document that the USB descriptor implementation currently supports only 9 axes; 16-axis support is not implemented and hardware integration is still pending.
- Polish the PC-host build/system simulator, add more simulator functionality, and improve its integration with the actual hardware workflows.


