# Universal Button Matrix

UniversalButtonMatrix is a configurable Raspberry Pi Pico firmware for large button matrices. It uses shift registers to expand button inputs while minimizing MCU GPIO usage.

The design requires only a few output pins to steer the shift registers and a smaller number of input pins to read back the matrix state. For example, 4 output pins plus 1 input pin can scan 24 or more buttons, and the system scales by chaining additional shift register stages. This keeps the Pico's native GPIOs available for other peripherals while supporting larger button arrays.

## Firmware

This firmware supports three build-time variants: `VARIANT_32_BUTTONS_ENABLED`, `VARIANT_64_BUTTONS_ENABLED`, and `VARIANT_128_BUTTONS_ENABLED`. Exactly one variant must be enabled to set the matrix dimensions and HID report size.

The 32-button variant has lower latency than the larger variants, since it uses fewer columns and rows.

The build also supports a host simulation mode via `HOST_BUILD`. When `HOST_BUILD=ON`, the code compiles a PC-side tester instead of Pico firmware, allowing verification without actual hardware.

`CONSOLE_DEBUG` controls runtime debug output verbosity for the Pico build:
- `0`: no serial/CDC debug output
- `1`: minimal verbosity — displays core loop timing only, useful for worst-case latency diagnostics
- `2`: medium verbosity — shows a button-press console visualization
- `3`: maximum verbosity — not used yet

**About PicoBoard RP2040-Zero**
![](doc/RP2040.png)
The software is designed primarily for PicoBoard RP2040-Zero, but it is easy to adapt to other Raspberry Pi compatible boards.

Used pinouts:
| *RPi-GPIO* | *Assignment* |
| --- |---:|
| 27 | LATCH |
| 26 | REGISTER CLEAR |
| 15 | DATA OUTPUT |
| 14 | CLOCK |
| 4, 5, 6, 7, 8 | ROW 1-5 INPUT |
| 9, 10, 11, 12, 13 | ROW 6-10 INPUT |

**About matrix keyboard**

**About shift register board**
More columns require more RP2040 bit shifting, which increases scan latency.

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

**Docker (Windows) build**

This repository includes a Dockerfile at `UBM-PicoBuild.dockerfile` for any-OS-docker-based Pico builds.

Build the Docker image from the repository root (Windows example):

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

## TODOs:
- Document that the USB descriptor implementation currently supports only 9 axes; 16-axis support is not implemented and hardware integration is still pending.
- Polish the PC-host build/system simulator, add more simulator functionality, and improve its integration with the actual hardware workflows.


