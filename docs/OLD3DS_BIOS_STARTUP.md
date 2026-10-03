# Native BIOS IRQ policies and the GT2 DMA boundary

Historical checkpoint: commit `64b5e5d` passed the user's original Old 3DS test
at 20 guest entries / 1,307,514 instructions. The current build advances further;
see [native DICR and current hardware expectations](OLD3DS_DMA_INTERRUPTS.md).
The results below record the earlier BIOS-policy milestone.

## BIOS behavior and references

`native/runtime/startup_services.cpp` implements B(5Bh) ChangeClearPAD as a
32-bit flag exchange: A0 supplies the new pad/card VBlank auto-ack policy, V0
receives the previous flag, and execution returns to guest RA. Zero disables
automatic acknowledgement; nonzero enables it. Only V0 and the execution PC
change; the other guest registers, HI/LO and COP0 state are preserved. The call
does not acknowledge an interrupt, change I_MASK, poll a pad or run a callback.

The policy applies to IRQ0 (VBlank), not IRQ7 (serial pad/card IRQ). The separate
`acknowledge_pad_vblank` helper implements its eventual handler-side action:
when enabled, acknowledge only IRQ0 through the native interrupt controller.
When disabled, leave every pending bit untouched so subsequent handlers can
process VBlank. The boot probe does not invoke this helper: no pad handler or
asynchronous IRQ scheduling is implemented yet. Tests exercise it directly.

The [PsyQ library reference, ChangeClearPAD, page 1-12](https://psx.arthus.net/sdk/Psy-Q/DOCS/LibRef47.pdf)
describes the enable/disable policy and declares a void SDK interface.
[PSX-SPX's pad auto-ack patch notes](https://psx-spx.consoledev.net/ps1/kernelbios/patches/)
identify the affected IRQ as VBlank.
[OpenBIOS's SIO0 driver at b8a9080](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/sio0/driver.c)
provides the lower-level flag exchange/old-value return and nonzero test. Our
bounded state starts with its cold SIO0 flag value of zero; driver start would
enable it. This is an explicit probe initialization policy, not a claim that
every retail BIOS/disc-loader handoff has identical driver state. GT2 explicitly
sets zero here, making subsequent behavior independent of the incoming flag.

The next observed call is C(0Ah) ChangeClearRCnt(3, 0). It exchanges the flag for
one of four BIOS handlers: timer 0, 1, 2, or VBlank (index 3), returning the old
flag in V0. The native state uses four fixed words initialized to one, modeling
the initialized BIOS timer-handler policies. GT2 changes index 3 from one to
zero; the other three remain one. Invalid indices remain unresolved instead of
indexing outside that array. Hardware timer registers and I_MASK are unaffected.
This follows [PSX-SPX's timer BIOS specification](https://psx-spx.consoledev.net/ps1/kernelbios/timer-functions/)
and [OpenBIOS's timer handler initialization and setter](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/handlers/irq.c).

The existing desktop `BiosB.cs` returns zero for B(5B), and `BiosC.cs` does nothing
for C(0A). Neither models the policy. They were inspected but not copied as
successful implementations. Existing native B(19h) HookEntryInt remains intact.
No reference implementation source was copied into the repository.

## Authentic startup progress

The executable is still the validated SCUS_944.88 Simulation revision, entry
0x8005D600, GP 0, stack 0x801FFF00. Its original B(5B) wrapper at 0x8008C998
returns to 0x8008C178 with A0=0. The added C(0A) wrapper at 0x8008CC58 returns
to 0x8008C184 after A0=3/A1=0. GT2 restores I_MASK to 1 and returns from its
VBlank callback-registration routine, then enters its DMA callback setup.

New static-recompilation ranges (end exclusive):

| Range | Role |
|---|---|
| 8008CC58..8008CC64 | Original C(0A) BIOS tail-call wrapper |
| 8008C668..8008C6B4 | DMA callback setup |
| 8008C8E0..8008C904 | Original eight-word callback-table clear loop |

The next substantive unresolved device is **DICR, 0x1F8010F4**, the DMA interrupt
control register. GT2 attempts a 32-bit zero store at **PC 0x8008C698**, in the
delay slot of JAL at 0x8008C694. The runtime reports an unmapped-memory trap with
BD set and EPC 0x8008C694. It does not silently discard that write. There are
**20 guest entries and 1,307,514 attempted guest instructions**, 90 more than
the previous checkpoint. The final faulting store is included in that count.
The last successful instruction is 0x8008C694; the most recently entered guest
routine is the clear loop at 0x8008C8E0, which has returned to DMA setup.

Three native BIOS calls were handled: HookEntryInt, ChangeClearPAD and
ChangeClearRCnt. BIOS transitions consume watchdog budget but do not invent
MIPS instruction counts. No new MIPS operations were needed. Original guest
instructions and their branch/load delays remain generated C++ compiled for ARM.
The shared 2 MiB RAM, 1 KiB scratchpad, constant executable image, 32-entry trace
and 2,000,000-step budget remain bounded. New BIOS state is a few fixed words.

The next implementation step after hardware validation is a minimal native
DICR implementation with its DMA IRQ enable/flag/acknowledge behavior, followed
by another run of this exact startup path. No DMA transfers, graphics, audio,
pad polling or game menus are part of this milestone.

## Reproduce and validate

From the repository root, using the already validated local IMG/CUE:

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py --boot
```

Run host tests in a Visual Studio compiler prompt, or supply `--compiler` for a
C++17 compiler. Content-free and hash-only configurations remain supported via
`python tools/test_native_guest.py` and `--gt2-probe`. All three passed MSVC
/O2 /W4 /WX. Four new BIOS tests cover flag exchange/ABI preservation, selective
VBlank acknowledgement, all timer-policy indices/old returns, reset and rejected
calls. The same BIOS suite runs on 3DS alongside MIPS 16/16 (`5775a33b`), GT2 hash
6/6 (`2ceef133`), boot runtime 2/2 and IRQ 5/5.

Host integration checks preserve the no-device I_MASK boundary, 32-instruction
watchdog and HookEntryInt checkpoints. Additional budget checkpoints stop just
before/after both new calls, including the former 17-entry B(5B) boundary.
The boot PASS predicate verifies exact counts, flags, MMIO activity, hook buffer,
the original BSS/heap initialization, and the DICR delay-slot trap.

Build in devkitPro: `make -C platform/3ds WITH_GT2_BOOT=1`. PowerShell fallback:

```powershell
& C:/devkitPro/msys2/usr/bin/bash.exe -lc 'cd "$(cygpath -u "$1")" && make -C platform/3ds WITH_GT2_BOOT=1' build-3ds (Get-Location).Path
```

Output: **`platform/3ds/opengtps1-old3ds-bootprobe.3dsx`**. Replace the previous
SD-card copy and launch with Homebrew Launcher. The local binary embeds the
validated executable; disc files are not needed on the SD card. Generated code,
disc/extracted data and binary artifacts stay ignored and uncommitted.

The devkitARM build completed without warnings; devkitPro `3dsxdump` accepted
the 860,792-byte 3DSX (49 code / 157 rodata / 2 data / 516 BSS pages).
SHA-256: `e9ca1606c8c368789ad6ebaa420dc14b7843105b2ff20e21b7b88824cff81225`.

## Expected bottom screen

```text
OpenGTPS1 / Old 3DS BIOS probe
START exit / X rerun / A color
Y: synthetic test details
MIPS 16/16 hash 5775a33b
GT2 hash 6/6 sig 2ceef133
Boot runtime 2/2 / IRQ 5/5
BIOS 4/4 / handled: 3
GT2 BOOT PASS (bounded stop)
Entry       8005d600
PC          8008c698
Last OK     8008c694
Last entry  8008c8e0
Guest entries 20
Instructions 1307514
I_STAT R:0 W:1
I_MASK R:2 W:3
I_STAT crossed: PASS
PAD ack OFF / calls: 1
VBL ack OFF / calls: 1
Unresolved  1f8010f4
Boundary: DICR (DMA IRQ control)
Trap: unmapped memory
BSS PASS / DMA W:1 / Timer W:1
Recent startup entries:
 16: 8008c0b4
 17: 8008c998
 18: 8008cc58
 19: 8008c668
 20: 8008c8e0
```

PASS means the expected bounded checkpoint was reached, not that GT2 finished
booting. A changes the top-screen color while held; X reruns; Y shows synthetic
details; START exits. A brief bounded startup pause is expected on launch/rerun.
