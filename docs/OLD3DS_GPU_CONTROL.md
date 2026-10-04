# Native GP1 display control and the CdInit BIOS boundary

This continues validated commit `28eb1e4` on `old3ds-port`. The user reports
that all diagnostics at that checkpoint pass on physical Old 3DS. This new
checkpoint has passed host tests and Old 3DS build validation; its physical
hardware run is pending.

GT2 now executes all four original VBlank callbacks, writes GP1 four times,
returns through BIOS B(17) four times, and removes its temporary callback.
The next authentic blocker is **BIOS B(3Fh), puts**, at **000000B0**, called
from the original CdInit initialization path. It is deliberately unresolved.
No guest instruction, polling loop, callback, or BIOS output is skipped.

## Implemented device scope

`GpuControl` accepts aligned 32-bit GP1(03h) display-enable commands at
1F801814, including the documented command-byte mirrors. Parameter bit 0
controls GPUSTAT bit 23; the unused parameter bits do not change other state.
Reads return status, never the previous command word. Memory routes physical,
KSEG0 and KSEG1 accesses through the same device.

The bounded GPU fixture starts in reset drawing/display state with an empty
command FIFO, DMA disabled, no GPU IRQ and no pending readback. Idle readiness
bits therefore describe an empty device, not fabricated transfer completion.
Bit 13 reflects noninterlaced mode. Bit 31 derives from the existing fixed
NTSC clock, with phase zero at VBlank start (line 256), 263 lines per field,
4305 half CPU cycles per line, and reset visible lines 16..255. Display disable
does not halt this clock. Reads do not advance time.

This is an explicit reset-state loader fixture, not a reconstruction of a retail
BIOS's GPU state at executable handoff. The prior one-cycle-per-instruction
timing approximation remains unchanged. PAL, interlace, display-mode/range
changes, GPU drawing, VRAM, DMA transfers and GPU IRQ generation remain
unsupported. Every other GP1 command, GP0 access, narrow or misaligned device
access is rejected without mutation; guest execution stops at unsupported
accesses. Even GPU reset is not silently accepted as a substitute for a future
complete reset implementation. GT2 performs no GPU status reads on this path;
the status semantics are exercised by synthetic tests.

Hardware references:

- [PSX-SPX GP1 commands](https://psx-spx.consoledev.net/ps1/gpu/display-control-commands-gp1/)
- [PSX-SPX GPU status](https://psx-spx.consoledev.net/ps1/gpu/status-register/)
- [PSX-SPX BIOS puts](https://psx-spx.consoledev.net/ps1/kernelbios/tty-console-std-io/)

## Authentic execution evidence

The default watchdog is now 4,000,000 steps so four scheduled callbacks can
finish. The clock rate is unchanged. The explicit 2,000,000-step scheduled
regression completes one callback and waits with count 1. With scheduling
disabled, the original 2,000,000-step counter-zero wait still passes unchanged.

Only these additional original instruction ranges are translated (end exclusive):

| Range | Purpose |
|---|---|
| 80089F38..80089F50 | CdInit entry through its first call |
| 80089FC8..80089FD8 | CD initialization wrapper through its first call |
| 8008B6D8..8008B6F0 | Initialization entry through its first console call |
| 8008E00C..8008E018 | Original BIOS B(3F) wrapper |

The ranges add 19 executed guest instructions after the four-frame wait.
Their bytes come only from the locally hash-validated executable. Generated
code, the disc, extracted executable and binaries remain ignored.

| Observation | Value |
|---|---|
| Guest entries / instructions | 53 / 3,396,889 |
| PC / last successful PC / last entry | 000000B0 / 8008E014 / 8008E00C |
| Stop / BIOS API | unresolved BIOS / B(3Fh), puts |
| String argument / return address | 80090E94 / 8008B6F0 |
| GP1 writes / status reads / last command | 4 / 0 / 03000001 |
| GPU status at stop | 14802000 |
| Scheduler edges / half-cycle phase | 6 / 488 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| Temporary callback / in-interrupt flag | 0 / 0 |
| Wait polls | 417,691 |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 15/5 |
| I_STAT / I_MASK / SR | 000 / 009 / 00000401 |
| Cause / EPC | 00000000 / 80010974 |
| BIOS calls handled / SYS calls | 8 / 1 |
| DMA completions / IRQ rises | 0 / 0 |

BSS clears and heap initialization remain verified. The trace capacity is 64
so the full 53-entry trace, including the final CD wrappers, is visible.
`PASS (bounded stop)` means this exact expected unresolved boundary, not a
successful game boot, CD initialization or rendered frame.

## Validation and reproduction

From a Visual Studio compiler environment (or supply `--compiler` explicitly):

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py
python tools/test_native_guest.py --gt2-probe
python tools/test_native_guest.py --boot
```

Host MSVC C++17 /O2 /W4 /WX validation passed: MIPS 16/16 (`5775a33b`),
GT2 hash 6/6 (`2ceef133`), boot runtime 6/6, IRQ 9/9, BIOS 8/8, DMA 6/6,
and GPU 5/5. Emitter tests and generated-fixture checks passed. Portable
platform I/O and geometry tests also passed with /W4 /WX.

GPU tests cover display enable/disable, ignored parameter bits, command
mirrors, status readback, all unsupported command bytes, width/address guards,
memory aliases, delay-slot fault preservation and visible-line/border transitions. Guest
integration checks bracket the original GP1 delay-slot write, verify the first
IRQ return, then verify four returns and the exact unresolved CdInit call.

devkitPro dependency check and forced rebuilds passed for data-free, GT2 hash
and GT2 boot variants. The boot ELF uses ARMv6K/VFPv2; no New 3DS options.
`3dsxdump` accepted 61 code / 158 rodata / 2 data / 516 BSS pages.
ELF text/data/BSS: 892,152 / 7,608 / 2,114,336 bytes (excludes runtime heaps).

Copy `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` and its `.smdh` over the
previous SD-card probe. No disc files are needed on the SD card. The 3DSX is
915,656 bytes, SHA-256:
`e6544af7f162b4991dd229e1879321f9a408c13ac30453848f616b5e1fe12573`.

Expected screen: `GPU 5/5`, `GT2 BOOT PASS (bounded stop)`, PC `000000b0`,
53 entries, 3396889 instructions, `GP1 W:4 status:14802000`, four IRQ entries
and returns, VBlank count 4/4, and `Boundary: BIOS B(3f) puts`.
START exits, X reruns, Y shows test details, A changes the host clear color.

Next work begins with the unresolved BIOS console operation. No console output
has been discarded or acknowledged by this checkpoint; CD device access remains
unreached and unimplemented.
