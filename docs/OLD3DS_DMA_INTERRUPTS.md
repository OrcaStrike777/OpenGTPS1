# Native DICR and the next GT2 startup boundary

Status: the preceding BIOS milestone, commit `64b5e5d`, passed on an original
Old 3DS. This DICR milestone passes host tests, builds for ARM11 and passes
devkitPro's 3DSX container check. Its new hardware run is pending.

## Register behavior

`native/runtime/ps1_dma.cpp` adds an explicitly attached, bounded DICR device
at 0x1F8010F4. Its initial/reset register, counters and IRQ output are zero.
DPCR remains separate with its existing reset value of 0x07654321.

* Bits 16..22 enable channel 0..6 completion interrupts; bit 23 is master enable.
  `complete(channel)` latches bit 24+channel only when both enables are set.
  Enabling them later does not replay a masked completion.
* Flags 24..30 persist until a write-one acknowledgement; writing zero retains
  them. Software cannot set these flags directly. Turning channel enables off
  does not hide already-latched flags from the master calculation.
* Read-only bit 31 is recomputed as `bit15 || (bit23 && any_channel_flag)` after
  writes and completions. Bit 15 is the writable bus-error/force-IRQ state.
* The resulting level drives IRQ3 in the existing interrupt controller. Rising
  edges latch I_STAT even if I_MASK is zero. A held level does not repeatedly
  latch IRQ3 after I_STAT is acknowledged. DICR acknowledgement/reset does not
  clear I_STAT; software must acknowledge both registers independently.
* Low channel policy bits 0..6 are retained; unused bits 7..14 read zero. There
  is no per-slice scheduling or transfer engine yet.

These rules follow the [PSX-SPX DMA hardware specification](https://psx-spx.consoledev.net/ps1/system/dmachannels/).
The controller has no buffers, unbounded arrays, deferred closures or host time.

8/16/32-bit aligned reads support physical/KSEG0/KSEG1 addresses. Ordinary
SB/SH/SW writes latch the full source word shifted by the byte offset, following
the [hardware-tested on-die MMIO bus behavior](https://psx-spx.consoledev.net/ps1/system/partialwordwrites/).
Partial writes do not merge with old readback, which could inadvertently clear
W1C flags. The guest emitter already passes the full source register to stores.
Invalid widths, misalignment, KSEG2 and neighboring addresses remain rejected.
GT2 uses a 32-bit SW on this path. MMIO-specific SWL/SWR bus behavior is outside
this milestone; the existing store-merge implementation/tests cover RAM.

## Desktop cross-check

The reference inspected is
`vendor/RecompOne/RecompOne.Runtime/Memory/Dma.cs`, particularly `WriteDicr`,
`Complete` and `Run`, plus the PSMemory MMIO routing and IRQ callback hookup.
The native code preserves its W1C algorithm, channel/master completion gating
and force/master-flag formula. There are deliberate corrections:

* Desktop `WriteDicr` updates bit 31 but does not signal `_raiseIrq` when a write
  activates it. Native writes and completions both update the IRQ3 line.
* Desktop `Complete` sets a flag and queues an IRQ, but does not update bit 31
  there. Native completion updates readable bit 31 immediately with that event
  and emits only a rising edge, including when multiple flags accumulate.
* Desktop retains every low-24-bit write. Native masks the unused field and
  implements ordinary partial-store bus behavior explicitly.

Desktop transfer engines and their allocations were not ported. The native
completion entry point is tested with deterministic device notifications, but
the real boot probe never synthesizes a completion. Channel register attempts
are counted and still trap rather than implying a successful transfer.

## Authentic execution

The supported SCUS_944.88 executable still enters at **0x8005D600** with the
original layout, GP and SYSTEM.CNF stack. Its SW at **0x8008C698** now succeeds
in the JAL delay slot. GT2 then runs its original IRQ3 callback registration,
restores I_MASK to **0x009**, returns from DMA setup and calls its next BIOS
wrapper at **0x8008CC90** (new generated range ends at 0x8008CC9C).

The probe stops at **BIOS A(72), vector 0x000000A0**, commonly named
RemoveCdromIrq / `_96_remove`. Last successful PC is **0x8008CC98**, the wrapper's
delay slot; return address is 0x8008BEC8. Counts are **23 guest entries** and
**1,307,596 guest instructions**, versus the hardware-validated 20 / 1,307,514
checkpoint. Entry count includes repeated calls, not just distinct routines.

This is a genuine unresolved BIOS CD-ROM/IRQ cleanup boundary. The desktop
BiosA implementation simply breaks for 0x72. The
[BIOS CD-ROM reference](https://psx-spx.consoledev.net/ps1/kernelbios/cdrom-functions/)
identifies the cleanup API and its interrupt-queue caveat. The native runtime
does not substitute that desktop no-op for modeled BIOS handler state. No new
BIOS calls or MIPS opcodes were added in this milestone.

Real guest activity: DICR **0 reads / 1 write**, final value **0**, DMA IRQ edges
**0**, completion notifications **0**, and channel-register reads/writes **0/0**.
DPCR still has one write. I_STAT has 0 reads/1 write, I_MASK 3 reads/5 writes,
final IRQ pending/mask 0/9. Three earlier BIOS calls remain handled.

The next concrete step is to model the BIOS CD-ROM interrupt-handler cleanup
needed by A(72), then continue the original startup path. No DMA transfer engine,
rendering, sound or pad polling is needed at this checkpoint.

## Build and verification

In a host compiler prompt from the repository root:

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py --boot
```

The content-free and hash-only host configurations also pass, using the same
test command without options and with `--gt2-probe`, respectively. MSVC /O2
/W4 /WX reports MIPS 16/16 (`5775a33b`), GT2 hash 6/6 (`2ceef133`), boot runtime
2/2, IRQ 5/5, BIOS 4/4 and **DMA 6/6**. The six DMA tests cover reset/read-only
bits, all channel/master gating combinations, flag persistence/W1C, force/IRQ3
edge and acknowledgement ordering, MMIO widths/aliases, and unmapped channels.
These same suites are compiled into and run by the 3DSX.

All earlier watchdog tests remain. Additional checkpoints stop immediately
before/after the original DICR delay-slot write, proving that it executes once
and advances to the original call target. The final PASS predicate verifies
the new BIOS boundary, counts, DICR/IRQ state and activity, previous BIOS flags,
hook buffer and original BSS/heap initialization. The execution budget remains
2,000,000 steps, with BIOS transitions charged separately from instructions.
RAM remains a shared 2 MiB arena plus 1 KiB scratchpad and a fixed trace.

Build in devkitPro with `make -C platform/3ds WITH_GT2_BOOT=1`, or PowerShell:

```powershell
& C:/devkitPro/msys2/usr/bin/bash.exe -lc 'cd "$(cygpath -u "$1")" && make -C platform/3ds WITH_GT2_BOOT=1' build-3ds (Get-Location).Path
```

Output: **`platform/3ds/opengtps1-old3ds-bootprobe.3dsx`**, 865,428 bytes.
SHA-256: `5e856cd80c823e2865b313ee5efc6d2868cde55846a2c72150466b1bfc2db5e8`.
The build completed without warnings. `3dsxdump` accepted 50 code / 157 rodata /
2 data / 516 BSS pages. Copy this file over the previous SD-card version and
launch in Homebrew Launcher; no disc files are needed on the SD card.

Source, tests and build instructions are committed. The local boot artifact
embeds the user's game executable and remains ignored alongside generated code
and extracted data, preserving the requirement not to commit copyrighted data.

## Expected Old 3DS screen

```text
OpenGTPS1 / Old 3DS DMA probe
START exit / X rerun / A color
Y: synthetic test details
MIPS 16/16 hash 5775a33b
GT2 hash 6/6 sig 2ceef133
Boot runtime 2/2 / IRQ 5/5
BIOS 4/4 DMA 6/6 / handled: 3
GT2 BOOT PASS (bounded stop)
Entry       8005d600
PC          000000a0
Last OK     8008cc98
Last entry  8008cc90
Guest entries 23
Instructions 1307596
I_STAT R:0 W:1
I_MASK R:3 W:5
IRQ pending:000 mask:009
DICR R:0 W:1 =00000000
DMA IRQ edges:0 completions:0
DMA channels R:0 W:0
Crossed I_STAT:PASS DICR:PASS
PAD ack OFF / VBL ack OFF
Unresolved  000000a0
Boundary: BIOS A(72) _96_remove
Trap: unresolved BIOS
BSS PASS / DPCR W:1 / Timer W:1
Recent startup entries:
 22: 8008c0b4
 23: 8008cc90
```

PASS denotes the expected bounded stop, not a completed game boot. A changes
the top-screen color while held, X reruns, Y shows the synthetic detail page,
and START exits. A brief bounded pause on launch/rerun is expected.
