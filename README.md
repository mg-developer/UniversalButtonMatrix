# UniversalButtonMatrix

## Build

Build instructions for both x86 host testing and the Raspberry Pi Pico target.

**x86 (host) build — quick testing**

- Requirements: `cmake`, a C++ compiler (MSVC or GCC/MinGW).
- Configure and build the host test target:

```bash
cmake -S . -B build -DHOST_BUILD=ON
cmake --build build --target UniversalButtonMatrix_host
./build/UniversalButtonMatrix_host.exe
```

- You can also use the included VS Code tasks and launch configs:
	- See [/.vscode/tasks.json](.vscode/tasks.json#L1) and [/.vscode/launch.json](.vscode/launch.json#L1).

**Raspberry Pi Pico build**

- Requirements: `cmake`, Pico SDK. Set `PICO_SDK_PATH` to your Pico SDK location.
- Configure and build the Pico firmware:

```bash
cmake -S . -B build -DHOST_BUILD=OFF -DPICO_SDK_PATH=/path/to/pico-sdk
cmake --build build --target UniversalButtonMatrix
```

Replace `/path/to/pico-sdk` with the path where you cloned the Pico SDK.

If you want more host-side testing (stubbing `board_*`/`tud_*` APIs), add host stubs and I can integrate them into the host build.
