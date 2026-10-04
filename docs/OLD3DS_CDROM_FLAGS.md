# Native CD-ROM interrupt flags and request-register boundary

This continues physically validated `a9fe862`, pushed to `origin/old3ds-port`.
The user reports that its Old 3DS validation passed without issue. `main` remains
at d9e6af5. The new checkpoint executes the original interrupt-flags read at
8008B764 and stops on a **byte write of zero to 1F801803 with bank 0 selected**,
at **PC 8008B80C**. That write targets HCHPCTL, the CD request/control register.

## Implemented behavior and limits

HINTSTS byte reads at 1F801803 in banks 1 and 3 return the five stored interrupt
bits with bits 5-7 set. Reads preserve the latch. With no pending event the value
is E0h. Bank-1 HCLRCTL byte writes clear only the selected low-five flag bits;
zero leaves them unchanged. Selective acknowledgement treats the encoded HC05
interrupt type bitwise, so clearing bit 0 of INT3 leaves INT2. Acknowledgement
does not clear the separate I_STAT latch.

The runtime starts with no pending CD flags and has no CD event producer.
Nonzero flags occur only in synthetic tests of acknowledgement semantics.
No command is accepted, no response is supplied and no completion or CD IRQ is
fabricated. Parameter and result FIFOs remain empty; their contents, command
queues, interrupt enables and IRQ-line delivery are outside this checkpoint.
Future command support must implement these together with acknowledgement's
FIFO/queue side effects. The stored flags alone are not a command engine.

HCLRCTL bits 5-7 (XA buffer clear, parameter FIFO clear and decoder reset) remain
unsupported. Such writes reject atomically before changing flags or counters.
Bank-3 writes target different hardware and remain unsupported. Bank-0/2 mask
reads, the bank-0 request write and other ports also remain unsupported. This is
byte-only runtime support, not a claim that real hardware rejects wider accesses.

Register definitions were checked against the
[PSX-SPX CD-ROM register source](https://github.com/psx-spx/psx-spx.github.io/blob/master/docs/cdromdrive.md).

## Original guest execution

The translated range 8008B6D8..8008B768 extends to 8008B6D8..8008B810
(end exclusive), adding 42 original instructions from the validated EXE. This
includes both sides of the existing flag-test branch. No guest instruction or
branch decision is replaced.

GT2 reads E0h at 8008B764, retires the load delay, masks with 7 and executes its
BEQ at 8008B770 plus the delay slot at 8008B774. The guest itself branches to
8008B7CC because there is no pending interrupt; its acknowledgement loop is not
taken. Consequently the authentic run performs zero HCLRCTL acknowledgements.
Synthetic tests exercise that register independently.

The following original stores set bytes at 800A7AE8/9/A to 02/00/00 and select
bank 0 at 8008B7FC. The SB at 8008B80C then attempts to write zero to HCHPCTL and
traps. That operation is left unresolved at this checkpoint.

| Observation | Value |
|---|---|
| Guest entries / attempted instructions | 58 / 3,397,037 |
| PC / last successful PC / last entry | 8008B80C / 8008B808 / 8008C0B4 |
| Unmapped access | 1F801803, bank 0, byte write 00 |
| Cause / EPC | 0000001C / 8008B80C (not a delay slot) |
| CD reads / writes / bank / status | 1 / 2 / 0 / 18 |
| CD flag reads / acknowledgement writes / pending flags | 1 / 0 / 00 |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 16/7 |
| I_STAT / I_MASK / SR | 000 / 00D / 00000401 |
| Scheduler edges / half-cycle phase | 6 / 784 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| GP1 writes / reads / status | 4 / 0 / 14802000 |
| BIOS calls / puts / printf / SYS | 10 / 1 / 1 / 1 |
| Console transcript | `CD_init:addr=800a7aec\r\n` (23 bytes) |

BSS clears, heap initialization and the guest CD callback slot remain verified.
Video timing, the IRQ path and diagnostic presentation are unchanged. Generated
guest code and executable data remain ignored.

## Validation

Host MSVC C++17 /O2 /W4 /WX checks cover data-free, GT2 hash and GT2 boot builds:
MIPS 16/16 (`5775a33b`), GT2 hash 6/6 (`2ceef133`), boot runtime 6/6, IRQ 9/9,
BIOS 16/16, DMA 6/6, GPU 5/5 and CD 8/8. Generated-fixture checks, emitter tests,
portable I/O and geometry tests are included.

CD tests exercise all 32 flag states against all 32 acknowledgement masks,
bank-1/3 read mirroring, non-destructive reads, reserved bits, repeated empty
acknowledgement, unsupported control bits/widths, byte-bus truncation, physical
and cached/uncached aliases, guest fault state and the separate I_STAT latch.
Boot watchdogs bracket the flag read, the original branch and the next request
write. The earlier index, BIOS, GPU, DICR and unscheduled VBlank checks remain.

Validation commands (MSVC developer environment for host checks; devkitPro
MSYS2 environment for the ARM builds):

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py
python tools/test_native_guest.py --gt2-probe
python tools/test_native_guest.py --boot
python tools/check_3ds_toolchain.py
make -B -C platform/3ds WITH_GT2_BOOT=1
make -B -C platform/3ds
make -B -C platform/3ds WITH_GT2_PROBE=1
```

The devkitPro dependency check, all three forced Old 3DS builds and boot 3DSX
parsing pass without compiler warnings. The CD object targets ARMv6K/VFPv2.
ELF text/data/BSS: 914,880 / 7,608 / 2,114,336 bytes, excluding runtime heaps.
`3dsxdump`: 66 code / 158 rodata / 2 data / 516 BSS pages.

Copy `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` and its `.smdh` over the
previous SD-card probe. Binary size: 938,496 bytes. SHA-256:
`143002d90678e31cd4b7a3c3f0955e786504d19ef38c2d21d64f780378dc068e`.

Expected Old 3DS highlights: `GT2 BOOT PASS (bounded stop)`, PC 8008b80c,
58 entries, 3397037 instructions, `CD 8/8 bank:0 R:1 W:2`, and boundary
`CD-ROM request (bank0)`. Physical validation of `034720d` passed without issue
on the user's Old 3DS; it is published on `origin/old3ds-port`. The next checkpoint
is documented in [CD-ROM Request behavior](OLD3DS_CDROM_REQUEST.md).
PASS confirms the exact unresolved boundary; CdInit has not finished and no CD
commands execute. Previously reported text tearing remains diagnostic UI behavior.
