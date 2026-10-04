# Native VBlank IRQ delivery and the GPU control boundary

The user has validated this checkpoint on physical Old 3DS. It is superseded
by [GP1 display control and four completed callbacks](OLD3DS_GPU_CONTROL.md).

This checkpoint continues `88e4fc2` on `old3ds-port`. The supported, validated
SCUS_944.88 now enters its original VBlank callback through the installed BIOS
exit hook and the original SDK IRQ handler. The next authentic blocker is a
32-bit **GP1 display-disable write to 1F801814**, value **03000001**, at
**8007F844**. The callback has not returned and the four-frame wait is not yet
complete. No GPU write is discarded or treated as successful.

## Timing and interrupt scope

`VBlankClock` is a bounded, fixed NTSC non-interlaced loader fixture. It counts
263 lines at nominal 2152.5 CPU cycles per line, retaining the fractional field
period in half-cycle units (1,132,215 half cycles per field). Phase starts at
zero at executable handoff. Edges latch IRQ0 even when masked; pending edges
coalesce in I_STAT. The clock never reads host wall time or the 3DS frame clock.

**CPU timing is provisional:** this boot probe charges one emulated CPU cycle
per attempted guest instruction. It does not model cache/memory stalls, multiply
latency, GPU scanline/display-range registers, PAL/interlaced modes, or the
retail loader's exact video phase. Counts below are reproducible under this
explicit fixture, not measurements of real PS1 cycle timing. No PC-specific
trigger, spin-loop replacement, host callback shortcut, or counter injection
advances the game.

The interrupt controller exposes pending IRQ2 in Cause; the new BIOS handoff
checks current IE and the CPU interrupt mask. It defers entry until both a
branch delay slot and any pending load have retired. That is a deliberate
bounded-runtime simplification, not a claim of precise R3000 interrupt latency.
It never drops a load or replays a branch with an already-written link register.

For the supported path, only IRQ0 is unmasked/pending and both BIOS VBlank
auto-ack policies are disabled by the original GT2 startup calls. The runtime
saves one fixed guest context, pushes the SR status stack, records EPC, and
loads the 12-word `HookEntryInt` setjmp block from guest RAM. V0 becomes one;
RA, SP, FP, S0..S7 and GP come from the guest's saved block. Execution resumes
at its saved PC, **8008BE74**, rather than directly calling a callback-table
entry. Hook memory is validated before changing the context. Unsupported
sources, auto-ack policies, missing/invalid hooks, or nested enabled IRQs stop
with `unresolved IRQ delivery` rather than claiming a complete BIOS event chain.

B(17h) `ReturnFromException` now restores the interrupted guest context and RFE
status while preserving the cumulative instruction count. It resumes the
interrupted PC, never the wrapper's RA; it does not acknowledge I_STAT itself.
Synthetic generated code verifies guest acknowledgement, register restoration,
and delivery of a second edge. The authentic run below stops inside the first
callback, so its B(17) return is still **unreached**.

Sources inspected:

- [PSX-SPX GPU timing](https://psx-spx.consoledev.net/ps1/gpu/timings/) for NTSC field/line timing.
- [PSX-SPX BIOS interrupt handling](https://psx-spx.consoledev.net/ps1/kernelbios/interrupt-exception-handling/) for HookEntryInt, status and return behavior.
- [OpenBIOS hook structures](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/kernel/handlers.c) and [timer/VBlank handlers](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/handlers/irq.c).
- [GP1 display-control specification](https://psx-spx.consoledev.net/ps1/gpu/display-control-commands-gp1/) for the observed command.

The desktop `Hardware/Interrupts.cs` skips the SDK dispatcher and calls a table
entry directly. Its `GT2Compat.WaitForInitialVBlanks` writes the counter from a
host loop. Neither shortcut is used by this native implementation. No reference
implementation source or proprietary guest bytes were added to tracked files.

## Observed guest path

The first two scheduled edges occur while CPU interrupts are disabled and are
cleared by GT2's original startup I_STAT write. The third edge occurs after
the callback is registered, at 1,698,323 guest instructions. IRQ entry resumes
the saved setjmp at 8008BE74 with V0=1; GT2 branches to 8008BEE4. Its handler
reads I_STAT and I_MASK, acknowledges IRQ0 at 8008BFA4, and calls its installed
IRQ0 routine at 8008C5A0. That routine increments the SDK VBlank count and
iterates the original eight-slot callback table, reaching slot 4 at 80010928.
The callback calls 8007F830 with A0=1. That routine attempts the GPU write in
the JR RA delay slot at 8007F844, where execution stops.

Additional translated ranges (end exclusive):

| Range | Role |
|---|---|
| 8008BEE4..8008C0B4 | Original SDK interrupt handler |
| 8008C5A0..8008C60C | Original IRQ0/VBlank callback-table dispatcher |
| 80010928..80010954 | Original temporary GT2 startup callback |
| 8007F830..8007F848 | GPU display mask command wrapper |
| 8008CCA8..8008CCB4 | Original B(17) return wrapper, translated but not reached |

| Observation | Value |
|---|---|
| PC / last successful PC | 8007F844 / 8007F840 |
| Unmapped address / attempted value | 1F801814 / 03000001 |
| Stop / Cause / EPC | unmapped memory / 8000001C (BD set) / 8007F840 |
| Guest entries / attempted instructions | 31 / 1,698,433 |
| Scheduler edges / half-cycle phase | 3 / 221 |
| IRQ entries / returns / active | 1 / 0 / yes |
| Interrupted PC / BIOS hook PC | 8001096C / 8008BE74 |
| IRQ source pending / I_MASK / current SR | 000 / 009 / 00000404 |
| I_STAT reads/writes / I_MASK reads/writes | 1/2 / 4/5 |
| SDK VBlank count / GT2 wait count | 1 / 0 of 4 |
| GT2 wait polls / guest in-interrupt flag | 78,136 / 1 |
| BIOS calls / explicit SYS calls | 4 / 1 |
| DICR reads/writes / completions / IRQ edges | 0/1 / 0 / 0 |

IRQ entry consumes one watchdog step without inventing a guest instruction.
The 2,000,000-step limit remains; this run stops earlier at a hardware access.
Entry counts still mean visits to the generator's range starts; the mid-range
setjmp resume has its own `irq_hook_pc` diagnostic. BSS clearing and heap setup
remain verified. The saved context is one bounded object, with no recursion or
heap allocation. Unsupported GPU, CD-ROM, DMA transfer, sound and pad polling
remain explicit boundaries.

The next implementation is minimal native GP1 display-mask state with correct
GPU status/readback semantics, then rerun this same callback until it can
return naturally. The remaining three VBlanks and `CdInit` are still unproven.
The GPU write is encountered before GT2 increments 80011DF4; zero is the correct
wait counter at this checkpoint, not evidence of failed IRQ delivery.

## Reproduce and test

```text
python tools/prepare_native_gt2_boot.py --image "PATH/TO/SUPPORTED.img"
python tools/test_native_guest.py --boot
python tools/test_native_guest.py
python tools/test_native_guest.py --gt2-probe
make -C platform/3ds WITH_GT2_BOOT=1
```

Use a compiler shell or pass `--compiler` with the full C++ compiler path.
The disc and executable hashes are revalidated by the generator. The original
unscheduled wait remains testable with `run_boot_probe(true, 2000000, false)`;
it still reaches 80010974 / 27 entries / 1,999,995 instructions. The no-device
I_MASK trap, 32-step watchdog and all prior BIOS/DICR checkpoints are retained.

The host suites pass MSVC C++17 /O2 /W4 /WX: MIPS 16/16 (`5775a33b`), optional
GT2 hash 6/6 (`2ceef133`), boot runtime 6/6, IRQ 9/9, BIOS 8/8 and DMA 6/6.
New integration checkpoints bracket the scheduled edge, BIOS hook handoff and
faulting GPU delay-slot write. Tests cover fractional clock carry, coalescing,
masked requests, saved hook registers, deferred branch/load retirement,
unsupported delivery guards, generated guest acknowledgement/B(17) return,
and repeated delivery after return. The generated-fixture and emitter checks
also pass. Physical Old 3DS validation of this new checkpoint is pending.

The output remains `platform/3ds/opengtps1-old3ds-bootprobe.3dsx`; it embeds the
user's validated game executable and stays ignored with generated game code.
Copy it over the prior SD-card test binary. No disc files are needed on the SD.

The devkitARM rebuild completed without warnings. Binary size: **910,692 bytes**.
SHA-256: `f43e4c132945b113a0d1d631f5d9800ffd4e240420069802ce2e1a0f95c56da5`.
devkitPro `3dsxdump` accepted 60 code / 158 rodata / 2 data / 516 BSS pages.

Expected bottom-screen highlights:

```text
OpenGTPS1 / Old 3DS IRQ probe
Boot runtime 6/6 / IRQ 9/9
BIOS 8/8 DMA 6/6 / handled: 4
GT2 BOOT PASS (bounded stop)
PC          8007f844
Last OK     8007f840
Guest entries 31
Instructions 1698433
I_STAT R:1 W:2
I_MASK R:4 W:5
GP1 write 03000001 (blocked)
VBL edges:3 IRQ enter:1 ret:0
Callback:YES / IRQ active:1 SR:404
VBL count:0/4 SDK count:1
Unresolved  1f801814
Boundary: GPU GP1 display control
Trap: unmapped memory
```

PASS means the exact expected boundary, not a completed boot or a rendered
game frame. Controls remain START exit, X rerun, Y diagnostics, A host color.
