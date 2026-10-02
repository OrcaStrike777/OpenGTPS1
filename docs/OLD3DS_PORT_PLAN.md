# Gran Turismo 2 on Old Nintendo 3DS: initial port plan

Status: repository inspection and architecture plan, 2026-10-01.
No native game runtime or 3DS executable has been implemented or validated yet.

Implementation follow-up: the data-free native platform scaffold now exists;
see [build instructions and verification status](../platform/3ds/README.md).
The inspection below records the initial baseline, not current implementation
status. No GT2 guest runtime or hardware-validated executable exists yet.

## Branch and objective

Work exclusively on `old3ds-port`. At inspection it already existed, was checked
out with a clean working tree, and pointed to the same commit as `main`:
`d9e6af5dccf704383f28faf8d1b50633e2539046`. Preserve `main` at that commit.
Do not reset or recreate the existing branch.

The goal is playable GT2 on original/Old 3DS hardware using devkitARM and
libctru. OpenGTPS1's GT2 static recompilation, discovered entry points, overlay
handling, and compatibility work are the foundation. Desktop architecture and
the upstream fixed modern-renderer quality policy are not constraints for this
target. This is a substantial runtime and code-generation port, not a renderer
cross-compile. Playable performance remains unproven.

Start with the supported US Arcade disc (`SCUS-94455`) and one ordinary
menu-to-race flow. Add Simulation (`SCUS-94488`, revision 2) after that path is
functional and measured. Both remain part of the overall objective. Defer the
unified launcher, GT1 imports, external OGG music, stereo rendering, and desktop
enhancements. Require neither New 3DS features nor a higher CPU clock.

## What the repository actually provides

Paths below are relative to the repository root.

| Area | Evidence | Old 3DS decision |
| --- | --- | --- |
| Disc preparation and discovery | `tools/prepare_reference.py`, `tools/prepare_arcade_reference.py`, `tools/psx_iso.py`, `tools/gt2_vol.py` | Reuse matching-disc extraction, overlay tables, additional function entries, and patch knowledge on the development PC. |
| Recompiler | `vendor/RecompOne/RecompOne.Recompiler/Analysis/`, `Disasm/`, `CodeGen/InstructionEmitter.cs`, `CodeGen/FunctionEmmiter.cs`, `CodeGen/OverlayWriter.cs`, `CodeGen/EntryWriter.cs` | Retain analysis; add a C/C++ output backend. Existing function output explicitly uses C# `CpuContext`, `IMemory`, delegates, and managed hooks. |
| Generator dependency | `RecompOne.Recompiler.csproj` references `RecompOne.Runtime.csproj` | Separate the generator's necessary shared types from the desktop runtime dependency graph. .NET is acceptable as an offline build tool. |
| Guest state and memory | `RecompOne.Runtime/Context/CpuContext.cs`, `Memory/PSMemory.cs`, `Memory/MemoryMap.cs`, `Memory/Dma.cs` | Port a compact native register context, address mapping, devices, and DMA. Remove presentation provenance tracking from the initial CPU hot path. |
| Dispatch and overlays | `RecompOne.Runtime/Dispatch/Dispatcher.cs`, `IOverlay.cs`, `RelocatedMemory.cs` | Replace managed maps/delegates with generated native dispatch tables keyed by active overlay and guest address. Preserve relocation and callback behavior where required. |
| GT2 and SDK replacements | `RecompOne.Runtime/sdk/GT2Compat.cs`, `LibCd.cs`, `LibCdStream.cs`, `LibGpu.cs`, `LibEtc.cs`, `LibPad.cs` | Port the reachable correctness hooks; audit each patch before retaining, dropping, or rewriting it. |
| BIOS and hardware | `RecompOne.Runtime/Bios/`, `Hardware/Gte.cs`, `Timers.cs`, `Interrupts.cs`, `Spu.cs`, `XaAudio.cs`, `Mdec.cs`, `MemoryCard.cs`, `Controller.cs` | Reimplement the necessary HLE/device semantics natively, using current code as the behavior reference. |
| CD and loose files | `RecompOne.Runtime/Cdrom/`, `tools/prepare_arcade_loose_install.py`, `tools/recompone.arcade.loose.json` | SD-backed seek/read with bounded caches, preserving virtual LBA, sector, callback, and completion semantics. |
| Desktop presentation | `RecompOne.Runtime/Host/Window/`, `Gpu/Hle/D3D11GpuBackend.cs`, `Host/InputManager.cs`, `Host/Audio.cs` | Replace with libctru lifecycle, HID, timing, filesystem, and audio adapters plus a Citro3D graphics backend. |
| Existing native renderer | `native/CMakeLists.txt`, `native/src/world_gpu_renderer_d3d11.cpp`, `native/include/opengt/` | Reuse suitable geometry/material knowledge and fixtures selectively. The current graph includes D3D11 implementation and a shared live-renderer bridge; it is not a native game runtime. |
| Managed/native bridge | `RecompOne.Runtime/Gpu/Hle/LiveWorldRenderer.cs`, `native/src/live_renderer_bridge.cpp` | Replace P/Invoke, worker-thread assumptions, and frame serialization with bounded in-process native command submission. |
| Windows packaging | `tools/build.ps1`, `tools/unified-host/GranTurismo2PC.csproj` | Create an independent 3DS build. The existing pipeline compiles generated C#, builds a renderer DLL, and publishes a self-contained `win-x64` program. |

The runtime project targets .NET 10 and references NVorbis, ImGui.NET,
Silk.NET Input/Maths/Windowing/SDL, and Vortice Direct3D11/D3DCompiler/DXGI.
None belongs in the on-device dependency graph. Keep desktop tools available
as references where useful; preserving their architecture is not a port goal.

`tools/apply_gt2_enhancements.py` and `tools/apply_gt2_arcade_enhancements.py`
also need auditing: generated-source rewrites and guest patches cannot simply
be applied unchanged to a new native backend. Move required fixes into explicit,
backend-independent metadata or named native hooks.

## Native architecture

Proposed layout, not directories implemented by this planning change:

```text
native/runtime/             portable guest state, dispatch, BIOS, devices, GT2 hooks
native/platform/3ds/        libctru host and Citro3D presentation
native/platform/headless/   native differential-test host
platform/3ds/Makefile       separate devkitARM build and .3dsx packaging
generated/old3ds/           ignored C/C++ output and dispatch tables
tools/prepare_old3ds.py     offline extraction/validation and target configuration
```

Pipeline: matching user disc -> existing analysis plus audited GT2 patches ->
native source and overlay tables -> devkitARM static link -> `.3dsx` plus
prepared files on SD. Retain the existing C# backend as a differential reference.
Do not attempt to ship CLR, Mono, JIT, or the Windows executable on the device.

Use explicit boundaries for platform input, presentation, audio submission,
time, logging, and storage. Guest addresses remain 32-bit integers rather than
host pointers. Generated functions operate on a native guest context and memory
bus; direct calls are permitted only when overlay identity and target lifetime
are known. Indirect calls must honor the currently loaded guest overlay.
Initially link native overlay functions statically while loading guest overlay
data as needed; measure total text size early before assuming this fits.

Translate arithmetic with defined wraparound and explicit signedness. Test
delay slots, zero register, HI/LO, division edge cases, unaligned loads/stores,
branch targets inside delay slots, jump tables, and GTE saturation/flags.
Investigate load-delay behavior against the existing implementation and MIPS
semantics. Avoid C++ undefined overflow, invalid shifts, and unaligned pointer
dereferences on ARM.

GT2's coroutine continuations and longjmp replacements are a separate critical
work item. The Arcade preparation script explicitly discovers continuation
entries, and `GT2Compat.cs` uses exception-based transfers. Use explicit native
dispatch/unwind results or a carefully bounded trampoline; do not mechanically
replace managed exceptions with host `longjmp` across C++ objects.

## Old 3DS resource policy

- Start single-threaded for guest execution. Do not assume a second unrestricted
  application core or use desktop renderer worker scheduling unchanged.
- Measure the actual heap, linear-memory availability, code size, stack high-water
  mark, and allocation failures in the chosen launch environment on Old 3DS.
  Total physical RAM is not the application's memory budget.
- Initially reserve the runtime's existing 8 MiB devkit guest RAM configuration
  when retaining patches that depend on it. `MemoryMap.cs` also supports 2 MiB
  retail RAM, but upstream documentation records imported-content allocations
  at `0x80200000` and `0x80400000`. Audit the selected patch set before shrinking
  guest RAM; dropping GT1 content alone is not proof that every dependency is gone.
- Account separately for guest RAM, 1 MiB PS1 VRAM shadow, SPU RAM, generated
  code/tables, stack, framebuffers, linear GPU/audio buffers, textures, and I/O.
  Establish hard cache caps after the first hardware memory probe. Never load a
  whole disc, volume, or unbounded resident course cache into RAM.
- Start with mono top-screen output and bottom-screen diagnostics. Preserve the
  authored aspect ratio; use PS1-scale rendering and bounded original-detail
  geometry before considering enhancements. Do not inherit mandatory 4x source
  rendering, maximum LOD, extended draw distance, or desktop antialiasing.
- Use a simple PS1 command renderer first: VRAM uploads/copies, textured sprites,
  triangles/quads, CLUTs, texture windows, draw areas/offsets, masks, and blending.
  Validate ordering-table behavior before introducing depth-based optimization.
  Use a CPU reference rasterizer for evidence, not as an assumed viable final
  racing renderer. Implement the shipping path using Citro3D/PICA200 with bounded
  texture conversion/cache invalidation and command batches.
- Preserve guest timing independently of display refresh. Measure guest CPU,
  GTE, rendering, I/O stalls, and audio costs separately. A provisional playable
  milestone is sustained 30 presented frames/s for a selected race at correct
  game speed; this is a target, not an established result or a change to guest
  VBlank semantics.
- Port SPU/XA mixing to bounded buffers and libctru NDSP output after silent boot
  is deterministic. Report unavailable audio initialization and permit silent
  diagnostics. Defer external OGG and full video playback, while retaining a
  correct path for skipping intros and implementing required MDEC operations.
- Map D-pad/Circle Pad, face buttons, Start/Select, and L/R through a native pad
  adapter. Define accessible L2/R2 bindings using touch or a modifier; do not
  assume New 3DS ZL/ZR. Handle suspend/resume and exit without corrupting saves.

## Implementation sequence and acceptance gates

1. **Toolchain and device probe.** Add the independent target using devkitPro's
   application template conventions and `3ds_rules`. Produce ELF/map/SMDH/3DSX;
   initialize libctru, display diagnostics, read HID, access an SD test file,
   report memory, and exit cleanly. Record exact toolchain versions and test on
   an actual Old 3DS. A booting shell is not a GT2 port.
2. **Native code-generation proof.** Add native output to the existing generator
   and compile small synthetic MIPS fixtures on a desktop headless host and with
   devkitARM. Compare registers, memory, and dispatch events with the C# runtime.
   Include indirect calls, overlapping overlays, continuation transfers, and
   GTE tests. Reject unsupported reachable operations rather than silently
   generating stubs. Measure compiled Arcade text/table size early.
3. **Minimal guest runtime and Arcade boot.** Port memory, dispatch, required
   BIOS/SDK calls, interrupts/timers/DMA, controller state, loose-file CD access,
   and necessary GT2 hooks. Record a patch inventory with guest address, overlay,
   reason, native implementation, and verification status. Reach the original
   Arcade title/menu with deterministic inputs and zero unexplained dispatch
   misses. Validate repeated overlay load/unload and asynchronous CD completion.
4. **Visible menu and one race.** Implement enough GPU command handling to
   navigate ordinary menus, choose a car/course, load a race, and drive with HID.
   Progress from captured command fixtures to live Citro3D submission. Complete
   a lap and the authored results flow before expanding content or visual scope.
5. **Playable hardware slice.** Add audio and robust memory-card persistence;
   verify save/reload, repeated races, suspend/resume, and a sustained run. Use
   bounded logs and frame-time distributions, not only average FPS. Profile
   Old 3DS bottlenecks and tune GTE, dispatch, texture uploads, batching, and SD
   reads. Consider further specialization only against measured hot paths.
6. **Simulation and coverage.** Port its distinct patches/overlays and prove
   purchase, upgrade, race, results, and save/reload. Broaden course/car coverage,
   video support, and compatibility before revisiting unified mode or extras.

Each gate should record input hashes, generated-source configuration, toolchain
version, hardware model/launch environment, memory peak, timings, and known
failures. Synthetic tests and capture fixtures can run without proprietary data;
full GT2 tests require the user's matching disc. Existing `tests/fixtures/` and
desktop capture tools are useful behavioral references, not proof of ARM success.

## Current limits and next concrete change

The inspected checkout has no `generated/` or `work/` directories. Python and
.NET executables are available; `arm-none-eabi-gcc`, `make`, and `cmake` were not
found on PATH, `DEVKITPRO`/`DEVKITARM` were unset, and `C:/devkitPro` was absent.
No game execution, native compilation, or on-device validation was performed.

Next implement gate 1 and its build/readme instructions, then the smallest
native emitter/runtime differential fixture from gate 2. Do not install a
toolchain or obtain game data as part of this planning-only change. Keep
extracted assets and generated game code ignored and locally produced.

## Toolchain references

Checked against primary project sources on 2026-10-01:

- [libctru](https://github.com/devkitPro/libctru): the supported library/toolchain
  relationship and 3DSX tooling.
- [Official 3DS application template](https://github.com/devkitPro/3ds-examples/blob/master/templates/application/Makefile):
  devkitARM build and packaging conventions.
- [devkitARM 3DS rules](https://github.com/devkitPro/devkitarm-rules/blob/master/3ds_rules):
  shared toolchain rules rather than manually invented packaging commands.
- [libctru SVC header](https://github.com/devkitPro/libctru/blob/master/libctru/include/3ds/svc.h):
  additional-core scheduling requires explicit application CPU-time handling.
