# Native boundary extraction: first implementation

Native guest milestone update (2026-10-02): the 3DS graph now also includes the
bounded guest runtime and shared synthetic C++ suite. `WITH_GT2_PROBE=1` adds
the locally generated string-hash function from the validated Simulation disc.
See [current native guest evidence](OLD3DS_NATIVE_GUEST.md). The original
platform-only extraction record follows.

The 3DS dependency graph is now independent of `tools/build.ps1` and the managed
host. `platform/3ds/Makefile` compiles only `main.cpp`, `host.cpp`,
`stdio_files.cpp`, and the existing `geometry_stitcher.cpp`; its libraries are
Citro3D, libctru, and the toolchain C/C++ runtime. No game assets are inputs.

`native/include/opengt/platform.hpp` defines six native boundaries: graphics,
input, audio, filesystem, timing, and saves. They use bounded buffers and
standard integer types; no libctru, D3D11, SDL, or managed types are exposed.
3DS SDK types are private to `native/platform/3ds/host.hpp` and its implementation.
Audio and save adapters explicitly decline unsupported operations. Device
emulation and guest timing belong above these host boundaries.

The unchanged geometry stitcher now lives in the `opengt_geometry` CMake target
under `native/portable`. The existing renderer links that target, avoiding a
forked implementation. This standalone portable project also builds the stdio
adapter and tests. It does not bring in capture serializers or the D3D11 renderer.
Other C++ code remains a reuse candidate, not automatically suitable for ARM
memory budgets simply because it compiles without Windows headers.

## Remaining desktop coupling

Paths under `vendor/RecompOne/` below are inspection evidence, not code already
ported. Managed source files were not changed by this extraction.

| Source | Coupling | Planned native destination |
| --- | --- | --- |
| `RecompOne.Runtime/RecompOne.Runtime.csproj` | .NET 10; Silk.NET, ImGui.NET, Vortice, NVorbis | No device dependency; behavior ported by subsystem. |
| `RecompOne.Runtime/Host/Window/` and `Gpu/Hle/D3D11GpuBackend.cs` | Windows graphics/window/UI and GPU presentation | `Graphics`; Citro3D PS1 command renderer still needed. |
| `RecompOne.Runtime/Host/InputManager.cs` | Silk.NET Input and SDL | `Input`; 3DS raw input implemented, PS1 mappings pending. |
| `RecompOne.Runtime/Host/Audio.cs` | SDL device output | `Audio`; future NDSP sink, separate from SPU/XA emulation. |
| `RecompOne.Runtime/Gpu/Hle/LiveWorldRenderer.cs` | P/Invoke DLL calls, managed worker, frame encoding | Direct bounded native rendering; no DLL in 3DS build. |
| `native/src/world_gpu_renderer_d3d11.cpp` | D3D11/compiler/WRL; desktop containers and threading | Excluded from 3DS; selectively reuse portable data/algorithms. |
| `RecompOne.Runtime/Runtime.cs` and `Host/FrameClock.cs` | Host pacing, managed global state, device scheduling | Split guest scheduler from `Timing` and application lifecycle. |
| `RecompOne.Runtime/Cdrom/` | Managed streams and manifests | `FileSystem` plus native virtual-sector/CD controller semantics. |
| `RecompOne.Runtime/Hardware/MemoryCard.cs` | Managed persistence and PS1 device state | Native protocol plus `Saves`; preserve failures and existing card images. |
| `RecompOne.Runtime/Dispatch/Dispatcher.cs` | Managed dictionaries/delegates and runtime hooks | Generated overlay-aware native dispatch tables. |
| `RecompOne.Runtime/sdk/GT2Compat.cs` | Managed hooks, exception-driven continuations, enhancement policies | Audit correctness hooks separately; native continuation trampoline. |
| `RecompOne.Recompiler/CodeGen/` | C# emission and managed entry point | Add native backend; do not string-rewrite generated C# into C++. |
| `RecompOne.Recompiler/RecompOne.Recompiler.csproj` | Direct dependency on the full runtime project | Extract offline shared disc/parser types before decoupling project reference. |

The generator's `Program.cs`, `Psx/Parser.cs`, and `Psx/SystemCfg.cs` import
`RecompOne.Runtime.Cdrom`. Generated-code strings referencing the runtime are
a separate issue from generator compile-time dependencies. Removing its project
reference without extracting the required CD types would break generation.
That managed build remains an offline reference for the forthcoming native
backend, not a proposed on-device CLR runtime.

The next extraction should introduce the native register/memory context and
small synthetic code-generation tests. Reuse RecompOne's analysis and preserve
delay-slot/overlay semantics before attempting the large GT2 compatibility
layer. This milestone does not claim that changing host interfaces makes the
existing managed game code runnable on 3DS.
