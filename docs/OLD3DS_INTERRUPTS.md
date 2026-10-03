# Native PS1 interrupt controller and GT2 startup checkpoint

Historical checkpoint, hardware-validated on an original Old 3DS on 2026-10-03:
17 guest entries / 1,307,424 instructions, stopping at B(5B) ChangeClearPAD.
The current build advances further; see [the BIOS startup milestone](OLD3DS_BIOS_STARTUP.md).
The expectations below record the preceding interrupt-controller milestone.

## Register behavior

`native/runtime/ps1_interrupts.cpp` implements the controller in bounded C++ state.
The temporary `BootIo` I_MASK latch and `attach_boot_io` shortcut are removed.
Devices remain explicitly attached to Memory; detached MMIO still traps, which
preserves a useful diagnostic and protects the existing synthetic RAM fixtures.

* I_STAT, 0x1F801070: 11 pending bits, IRQ0..IRQ10. Device requests latch even
  while masked. Reads return pending status without consuming it. Writes perform
  `pending &= value & 0x7FF`: zero acknowledges/clears, one preserves; software
  cannot create requests by writing status.
* I_MASK, 0x1F801074: reads/writes an independent 11-bit mask. Mask changes never
  discard pending requests. Bits outside 0..10 are ignored.
* Reset: status, mask, device input levels and access counters are all zero.
* `set_line(irq, asserted)` latches only rising edges. Acknowledging a held level
  does not immediately re-latch it; a new low/high transition does. `pulse(irq)`
  represents a one-shot request. Invalid source numbers are ignored.
* CPU IRQ signal is `(pending & mask) != 0`. `sync_cpu` updates COP0 Cause bit 10
  and preserves other Cause fields. This level is independent of SR interrupt
  enable; exception delivery/return and asynchronous device scheduling remain
  future work. No fake VBlank or host-time interrupts are injected into the probe.
* 16/32-bit accesses at each register base are supported through physical,
  KSEG0 and KSEG1 aliases. The unused upper halfword reads zero and writes have
  no effect. 32-bit reads have deterministic zero upper bits (hardware upper
  bits are unspecified); alignment, unsupported widths and KSEG2 trap.
* Successful register operations have separate I_STAT/I_MASK read/write counts.
  Rejected accesses do not increment them.

The desktop reference was inspected first: `RecompOne.Runtime/Memory/PSMemory.cs`
backs these two addresses with `_hwregs` storage, and `Hardware/Interrupts.cs`
delivers SDK callbacks directly. That is not an implementation of I_STAT's
write-zero acknowledgement or its edge latch, so copying it would be incorrect.
Register rules follow the [PS1 interrupt specification](https://psx-spx.consoledev.net/ps1/system/interrupts/).
The DMA reset value is corroborated by the [DuckStation DMA reset implementation](https://github.com/stenzek/duckstation/blob/master/src/core/dma.cpp),
and the priority-control layout is in the [DMA specification](https://psx-spx.consoledev.net/ps1/system/dmachannels/).

## Authentic progress and narrow additional ports

Entry remains 0x8005D600, load 0x80010000, GP=0 and SYSTEM.CNF SP=0x801FFF00.
The generator keeps the same disc/executable SHA-256 validation and ignored
local extraction/generation. It extends only the observed startup graph:

| Range (end exclusive) | Observed role |
|---|---|
| 8008BE0C..8008BEE4 | Existing interrupt initialization, extended after I_STAT |
| 8008C314..8008C338 | Original word-clear loop |
| 8007AD58..8007AD90 | Original setjmp/register save |
| 8008CC78..8008CC84 | Original B(19h) tail-call wrapper |
| 8008C548..8008C5A0 | Timer/VBlank callback initialization |
| 8008C638..8008C65C | Original callback-array clear loop |
| 8008BCA8..8008BCD8 | Original indirect library-table wrapper |
| 8008C0B4..8008C1FC | Original interrupt callback registration |
| 8008C998..8008C9A4 | Original B(5Bh) tail-call wrapper |

Three small additional pieces were needed along this path:

1. DMA **DPCR**, 0x1F8010F0: priority-control latch, initially 0x07654321,
   with bounded 16/32-bit reads/writes. GT2 performs its original 32-bit write.
   This follows PSMemory's DPCR storage behavior. DMA channels, transfers and
   DICR remain unmapped.
2. BIOS **B(19h) HookEntryInt**: register the original guest setjmp buffer,
   preserve registers and return through the guest RA. The desktop BiosB
   environment projection (`A0 - 0x36`) is retained. No callback executes.
3. Timer **mode setup writes**: configuration bits 0..9 and counter reset,
   mirroring the desktop `Hardware/Timers.cs` write behavior. GT2 writes timer 1
   mode at 0x1F801114. Counter reads, mode/status readback and timer clocks remain
   explicit unresolved operations; there is no wall-clock approximation.

All guest integer instructions in these added ranges use existing semantics.
No further MIPS operations were required. Original loops, setjmp, indirect calls,
load delays and branch slots execute through generated C++ without host function
substitutions. Native BIOS handling is separately counted from guest entries.

The new unresolved boundary is **B(5Bh) ChangeClearPAD** at BIOS vector **0xB0**.
The calling wrapper is 0x8008C998, its last executed slot is 0x8008C9A0, and the
return address into the original registration routine is 0x8008C178. GT2 is
installing the VBlank callback and has temporarily disabled I_MASK. The desktop
BiosB implementation merely returns zero for this API; it does not implement the
PAD/BIOS interrupt-clear policy. The native probe explicitly stops here rather
than marking that placeholder as a completed implementation. The next concrete
step after hardware validation is to implement this kernel policy and continue
original callback registration to its next observed dependency.

## Tests and build

```text
python tools/generate_boot_tests.py --check
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py --boot
```

Use a compiler prompt for the host tests (C++17; MSVC /W4 /WX). The same suites
are compiled into the 3DSX, including five new interrupt tests:

* Reset and all 11 masked pending bits
* Selective 16/32-bit acknowledgement, inability to create requests, edge rearming
* Mask/CPU Cause signal assertion and deassertion with other Cause bits preserved
* MMIO widths/aliases, invalid-access rejection and exact counts
* DPCR halfword merges, timer setup, BIOS registration/return and unknown-API rejection

The prior MIPS suite remains 16/16 (`5775a33b`), GT2 hash 6/6 (`2ceef133`), and
boot runtime 2/2 (store merges and opt-in register mapping). The old no-device
probe still stops at PC 0x8008BE44 / I_MASK, and a 32-instruction watchdog test
still stops at exactly 32. The default watchdog permits 2,000,000 execution steps (guest instructions and
native BIOS transitions). Tests also stop immediately before and after the hook
registration, proving that a BIOS dispatch consumes budget without inventing a
guest instruction. Thus even a malformed BIOS return loop cannot evade the limit.
There is one shared 2 MiB RAM arena, 1 KiB scratchpad, unchanged 626,688-byte
constant image and a fixed 32-entry trace. No new unbounded allocation exists.

Build in devkitPro:

```text
make -C platform/3ds WITH_GT2_BOOT=1
```

Windows PowerShell fallback:

```powershell
& C:/devkitPro/msys2/usr/bin/bash.exe -lc 'cd "$(cygpath -u "$1")" && make -C platform/3ds WITH_GT2_BOOT=1' build-3ds (Get-Location).Path
```

Output: `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` (same boot-target filename;
replace the previous copy). Copy to the SD card's `3ds` directory and launch in
Homebrew Launcher. Disc/extracted files are not needed on the SD card. Generated
proprietary code, extracted data and binaries remain ignored and uncommitted.

## Expected bottom screen

```text
OpenGTPS1 / Old 3DS IRQ probe
START exit / X rerun / A color
Y: synthetic test details
MIPS 16/16 hash 5775a33b
GT2 hash 6/6 sig 2ceef133
Boot runtime 2/2 / IRQ 5/5
GT2 BOOT PASS (bounded stop)
Entry       8005d600
PC          000000b0
Last OK     8008c9a0
Last func   8008c998
Functions   17
Instructions 1307424
I_STAT R:0 W:1
I_MASK R:2 W:2
I_STAT crossed: PASS
Unresolved  000000b0
BIOS API    B(5b)
ChangeClearPAD: unresolved
Trap: unresolved BIOS
BSS: PASS / BIOS handled: 1
DMA writes: 1 / Timer writes: 1
Recent startup entries:
 13: 8008c548
 14: 8008c638
 15: 8008bca8
 16: 8008c0b4
 17: 8008c998
```

PASS checks the expected bounded boundary, exact entry/instruction counts,
interrupt/DMA/timer access counts, hook buffer, original BSS clear and heap state.
It denotes progress to this checkpoint. Function count means guest entries
visited, not completed calls. The BIOS transition has no invented MIPS instruction
count; the unresolved vector is reported separately from the last executed slot.

A still changes the top-screen clear color while held, X reruns, Y toggles the
original synthetic details page, and START exits. No game scene, menus or sound
are presented. A brief bounded startup pause is expected during launch/rerun.
