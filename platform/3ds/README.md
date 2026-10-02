# Old 3DS bootstrap

This is a native platform bootstrap, not a playable GT2 build. It needs no game
data, BIOS, generated recompiler output, .NET, SDL, or Windows renderer DLL.
It targets original 3DS/2DS ARM11 hardware with devkitARM, libctru, and Citro3D.

## Build

Install devkitPro using its [official instructions](https://devkitpro.org/wiki/Getting_Started).
In its package-manager environment install the `3ds-dev` group (including
devkitARM, libctru, Citro3D, and 3DS tools):

```sh
dkp-pacman -S 3ds-dev
```

On Windows use `pacman -S 3ds-dev` inside the devkitPro MSYS2 shell. Build from
that shell, not plain PowerShell. `DEVKITPRO` and `DEVKITARM` must point to that
installation, and GNU make must be on PATH. For example, on Linux a standard
installation uses `/opt/devkitpro` and `/opt/devkitpro/devkitARM` respectively.
Use a checkout path without spaces for the GNU make build.

From the repository root:

```sh
python tools/check_3ds_toolchain.py
make -C platform/3ds
```

The read-only Python check is optional; Python is not a build dependency.
The Makefile uses the installed `3ds_rules`, compiles C++17 for ARMv6K, and links
Citro3D/libctru statically. Output alongside this README:

```text
opengtps1-old3ds.elf
opengtps1-old3ds.map
opengtps1-old3ds.smdh
opengtps1-old3ds.3dsx
```

Intermediate objects are in `platform/3ds/build/`. Outputs are Git-ignored.
Only explicit native source files are compiled; neither the desktop CMake
target nor RecompOne's managed runtime is linked. The existing geometry stitcher
is compiled unchanged for portability; unused code may be removed at link time.
The bootstrap does not yet render game geometry.

## Hardware smoke test

Copy the `.3dsx` and `.smdh` to `sdmc:/3ds/opengtps1/` and launch the `.3dsx`
using the Homebrew Launcher on an Old 3DS. Optionally place a small plain-text
`bootstrap.txt` in that directory; the application reads at most 64 bytes once.
It creates no files and never opens existing memory cards.

Expected behavior, **not yet verified on hardware**:

1. The top screen clears to blue; holding A changes it to green.
2. The bottom screen shows a frame counter, elapsed milliseconds, button masks,
   normalized Circle Pad axes, and available linear memory.
3. D-pad, A/B/X/Y, L/R, Select, and Circle Pad update the diagnostics. Button
   masks use `opengt::platform::Button`, not libctru's raw values. A pressed or
   released edge lasts one poll. Game/PS1 bindings have not been implemented.
4. `SD probe: ok` means the optional file opened/read successfully (an empty
   file also succeeds). Missing file/directory reports `unavailable` and does
   not prevent the app running. Other storage errors report `I/O error`.
5. START exits and releases the render target, Citro3D, and graphics subsystem.
   Initialization failures show an error and also permit START exit.
6. Check lid suspend/resume and launcher exit on hardware. `aptMainLoop()`
   handles the application lifecycle; elapsed time can include a suspension.

There is one explicit VBlank wait per loop. Presentation does not also use
`C3D_FRAME_SYNCDRAW`, avoiding a second pacing wait. This host clock does not
implement the future PS1 timer/interrupt scheduler. No New 3DS clock, extra-core,
stereo, or enhanced-memory mode is requested.

Record model, launcher environment, toolchain versions, linker text/data/BSS
sizes, displayed linear free memory, input results, and lifecycle results.
Linear free memory is **not** total free application heap or a peak-memory
measurement. Those probes and sustained hardware testing remain to be added.

## Portable code tests without a 3DS SDK

A separate CMake project builds the existing geometry stitcher and new bounded
stdio adapter without compiling any D3D11, SDL, or 3DS code:

```sh
cmake -S native/portable -B build/portable -DBUILD_TESTING=ON
cmake --build build/portable --config Release
ctest --test-dir build/portable -C Release --output-on-failure
```

Alternatively, in an x64 Visual Studio developer command prompt, from a newly
created `build/portable-check` directory:

```bat
cl /nologo /std:c++17 /EHsc /W4 /WX /D_CRT_SECURE_NO_WARNINGS /I..\..\native\include ..\..\native\platform\common\stdio_files.cpp ..\..\native\tests\platform_io_tests.cpp /Fe:platform_io_tests.exe
platform_io_tests.exe
cl /nologo /std:c++17 /EHsc /W4 /WX /I..\..\native\include ..\..\native\src\geometry_stitcher.cpp ..\..\native\tests\geometry_stitcher_tests.cpp /Fe:geometry_tests.exe
geometry_tests.exe
```

The I/O test creates/removes its own `platform-io-test.txt` in the working
directory. It tests offset reads, EOF, invalid buffers/paths, oversized offsets,
and explicit unsupported audio/save results. Use a disposable build directory.

## Current verification and limits (2026-10-01)

- Passed: portable I/O and existing geometry tests, compiled using local MSVC
  19.16 with C++17, `/W4 /WX`; `git diff --check`.
- Not run: CMake configuration (CMake not on PATH), ARM compilation/linking,
  3DSX packaging, emulator execution, or hardware smoke tests.
- Missing locally: `DEVKITPRO`, `DEVKITARM`, `arm-none-eabi-g++`, GNU make,
  `3ds_rules`, libctru headers/library, Citro3D headers/library, `3dsxtool`,
  `smdhtool`, and the toolchain's default SMDH icon. The dependency checker
  lists each missing item and exits nonzero.
- Audio deliberately returns `unsupported`. Memory-card load/store also return
  `unsupported`; there is no persistence or guest memory-card protocol yet.
- Read-only stdio opens/seeks/closes on each request. It bounds caller buffers
  and rejects offsets above `LONG_MAX`; it is not yet an optimized CD streaming
  backend. Path validation rejects traversal components but is not intended as
  a sandbox against host symlinks.
- Graphics currently clears/presents a target; no PS1 GPU, guest CPU, GTE,
  native generated functions, disc scheduler, or game execution exists yet.

Next: compile/package this target with devkitPro and run the smoke test on Old
3DS. Independently, implement a small native guest register/memory context and
the first C/C++ instruction-emitter fixture alongside the existing C# backend;
compare registers and memory after synthetic MIPS execution before adding GT2.

See [dependency separation](../../docs/OLD3DS_DEPENDENCIES.md) and the
[overall plan](../../docs/OLD3DS_PORT_PLAN.md).

SDK references used for the bootstrap:
[official application template](https://github.com/devkitPro/3ds-examples/blob/master/templates/application/Makefile),
[Citro3D example](https://github.com/devkitPro/3ds-examples/blob/master/graphics/gpu/simple_tri/source/main.c),
[libctru](https://github.com/devkitPro/libctru), and
[Citro3D render queue](https://github.com/devkitPro/citro3d/blob/master/source/renderqueue.c).
