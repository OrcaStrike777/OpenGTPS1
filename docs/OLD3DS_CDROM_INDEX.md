# Native CD-ROM index selection and interrupt-flags boundary

This continues physically validated `6f37885` on `old3ds-port`. The user reports
all diagnostics passing on Old 3DS, with 58 entries / 3,397,012 instructions at
the byte write of 1 to 1F801800, PC 8008B754. That commit is published on
`origin/old3ds-port`. `main` remains at d9e6af5.

The new checkpoint executes that write and the following original instructions,
then stops at **8008B764**, a **byte read of 1F801803 with CD bank 1 selected**.
That port is HINTSTS, the CD-ROM interrupt-flags register. It is the next
unsupported hardware behavior; no flag value or command result is fabricated.

## Implemented register behavior

Byte writes to 1F801800 retain bits 0-1 as the selected bank, initially zero.
Upper bits do not change the read-only status. Byte reads return the bank plus
18h: parameter FIFO empty and ready for a parameter, with no response, data
request, command busy or ADPCM busy. These bits describe the bounded controller's
idle state. No command is accepted, so no successful command or completion IRQ
is implied. GT2 has performed one index write and no supported CD reads at the
new boundary; status reads are exercised by synthetic tests.

The physical, KSEG0 and KSEG1 aliases use the same register state. Other
segments, ports 1F801801-1F801803 in every bank, and wider accesses remain
unsupported and preserve state. The byte-only scope is a runtime limitation,
not a claim that real PS1 hardware rejects wider bus transactions. Command,
parameter, response/data FIFO, interrupt-enable/acknowledgement, mixer, disc I/O
and command timing remain unimplemented. No CD IRQ is raised.

Register definitions were checked against the
[PSX-SPX CD-ROM register reference](https://psx-spx.consoledev.net/ps1/cdr/cdromdrive/).

## Authentic progress

The existing translated range 8008B6D8..8008B758 extends to 8008B6D8..8008B768
(end exclusive), adding four original instructions from the hash-validated EXE:

| PC | Original operation |
|---|---|
| 8008B754 | Existing SB selects CD bank 1 successfully |
| 8008B758 | LUI builds the address of the SDK register-pointer table |
| 8008B75C | LW loads 1F801803 from guest RAM at 800A7AD4 |
| 8008B760 | Original NOP retires the load delay |
| 8008B764 | LBU attempts to read bank-1 interrupt flags and traps |

No instructions, branches, calls or waits are bypassed. The previous boundary
count included the failed store; the new count includes the failed LBU.

| Observation | Value |
|---|---|
| Guest entries / attempted instructions | 58 / 3,397,016 |
| PC / last successful PC / last entry | 8008B764 / 8008B760 / 8008C0B4 |
| Stop / address / access | unmapped memory / 1F801803 / byte read |
| Cause / EPC | 0000001C / 8008B764 (not a delay slot) |
| CD index writes / supported reads / bank / status | 1 / 0 / 1 / 19 |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 16/7 |
| I_STAT / I_MASK / SR | 000 / 00D / 00000401 |
| Scheduler edges / half-cycle phase | 6 / 742 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| GP1 writes / reads / status | 4 / 0 / 14802000 |
| BIOS calls / puts / printf / SYS | 10 / 1 / 1 / 1 |
| Console transcript | `CD_init:addr=800a7aec\r\n` (23 bytes) |

BSS clears, heap initialization, the original callback execution and the guest
CD IRQ-handler slot (800A7B88 = 8008BA2C) remain verified. The provisional video
clock and diagnostic presentation timing are unchanged. Generated guest code
and executable data remain ignored.

## Validation and hardware expectations

Host MSVC C++17 /O2 /W4 /WX validation covers the data-free, GT2 hash and GT2
boot variants: MIPS 16/16 (`5775a33b`), GT2 hash 6/6 (`2ceef133`), boot runtime
6/6, IRQ 9/9, BIOS 16/16, DMA 6/6, GPU 5/5 and CD 5/5. Generated-fixture checks,
emitter tests, portable I/O and geometry tests are included.

CD tests cover all 256 index-byte values, read-only status bits, unsupported
widths and all banked ports, address aliases, and normal/delay-slot faults.
Boot watchdog checks bracket the index store and interrupt-flags load, checking
the bank, counters, exact PCs, instruction counts and fault state. Existing
no-device, unscheduled VBlank, BIOS, GPU and DICR checks remain included.

The devkitPro dependency check, forced builds of all three Old 3DS variants and
boot 3DSX parsing pass without compiler warnings. The CD object targets ARMv6K
and VFPv2. ELF text/data/BSS: 909,792 / 7,608 / 2,114,336 bytes, excluding runtime
heaps. `3dsxdump`: 65 code / 158 rodata / 2 data / 516 BSS pages.

Copy `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` and its `.smdh` over the
previous SD-card probe. Binary size: 933,404 bytes. SHA-256:
`34c774fa918544b61bdc950fe647bdee041fce5c8022ca02e0d2ffd926c9f89d`.

Expected highlights: `GT2 BOOT PASS (bounded stop)`, PC 8008b764, 58 entries,
3397016 instructions, `CD 5/5 bank:1 R:0 W:1`, and boundary
`CD-ROM IRQ flags (bank1)`. Physical validation of `a9fe862` passed without issue
on the user's Old 3DS; it is published on `origin/old3ds-port`. The next checkpoint
is documented in [CD-ROM interrupt flags](OLD3DS_CDROM_FLAGS.md).
PASS confirms the exact unresolved boundary; CdInit has not finished and CD
commands remain unsupported. Prior transient text tearing remains diagnostic UI behavior.
