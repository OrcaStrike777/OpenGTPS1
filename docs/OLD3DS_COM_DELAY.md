# Native COM_DELAY latch and Timer 1 counter boundary

This continues physically validated `b5470d8`, pushed to `origin/old3ds-port`.
The user reports that its Old 3DS validation passed without issue. `main` remains
at d9e6af5. The original delay-slot SW at 8008B820 now writes 00001325 to
1F801020 (COM_DELAY), and execution proceeds to its JAL target. The next
authentic blocker is a **32-bit read from 1F801110, Timer 1's current counter**,
at **PC 8008C35C**, inside the original `VSync(-1)` path called during CdInit.

## Register behavior and limits

COM_DELAY is a dedicated memory-control latch, independent of CD-ROM bank
selection. Aligned 32-bit writes replace its writable bits, and aligned 32-bit
reads return the stored value. Bits 0-15 contain the timing fields and bits
16-17 are also writable/readable; bits 18-31 read zero. Thus FFFFFFFF writes
read back as 0003FFFF, and writing 00001325 then reads back as 00001325, clearing
the previous upper writable bits. Repeated writes replace rather than merge
with prior state. Reads do not modify the latch.

The initial fixture value is the commonly used BIOS-style 00031125 configuration,
not a claim of a newly measured hardware power-on value. GT2 overwrites it
before any read. The current guest path performs one COM_DELAY write and no
COM_DELAY reads; synthetic tests validate readback independently.

Physical, KSEG0 and KSEG1 addresses share the same latch. Other memory-control
registers, subword accesses and invalid segments remain unresolved. These are
bounded implementation limits, not claims that real hardware rejects subword
access. Unknown accesses cannot mutate the latch or counters.

The register does not synthesize device activity: no CD command, sector data,
completion, response or IRQ is produced. Bus wait-state calculation remains
unmodeled; the existing provisional one-cycle-per-instruction video clock is
unchanged. This checkpoint implements register storage/readback, not cycle-exact
external bus timing.

The original [nocash PSX-SPX specification](https://problemkaputt.de/psx-spx.htm#memorycontrol)
describes bits 16-17 separately and bits 18-31 as zero on read. This corrects
the older mirror's statement that all bits 16-31 read zero. The mask and common
initial configuration were also checked against the
[DuckStation bus implementation](https://github.com/stenzek/duckstation/blob/master/src/core/bus.cpp).
No code was imported from that implementation.

## Original guest continuation

Three audited ranges are added from the hash-validated EXE, with end addresses
exclusive: 8008B034..8008B0FC, 8008AAEC..8008AB28 and 8008C338..8008C360.
These add 75 original instruction sites, of which 52 execute before the stop.
The guest's own branches decide the path; no instruction or call is skipped.

The SW at 8008B820 now retires without a fault, with RA=8008B824, then dispatches
the original target 8008B034. That code calls 8008AAEC, which calls 8008C338 with
A0=-1. VSync loads its register pointers from the original SDK table, reads
GPUSTAT at 8008C358 using the existing GPU model, then attempts the Timer 1 LW
at 8008C35C. Timer 1 counter reads remain unimplemented. This stop is not in a
delay slot, so Cause=0000001C and EPC=8008C35C.

| Observation | Value |
|---|---|
| Guest entries / attempted instructions | 61 / 3,397,094 |
| PC / last successful PC / last entry | 8008C35C / 8008C358 / 8008C338 |
| Unmapped access | 1F801110, 32-bit read |
| Cause / EPC / RA | 0000001C / 8008C35C / 8008AB28 |
| COM_DELAY value / reads / writes | 00001325 / 0 / 1 |
| CD reads / writes / Request writes / bank / status | 1 / 3 / 1 / 0 / 18 |
| CD flag reads / acknowledgement writes / pending flags | 1 / 0 / 00 |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 16/7 |
| I_STAT / I_MASK / SR | 000 / 00D / 00000401 |
| Scheduler edges / half-cycle phase | 6 / 898 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| GP1 writes / GPUSTAT reads / status | 4 / 1 / 14802000 |
| BIOS calls / puts / printf / SYS | 10 / 1 / 1 / 1 |
| Console transcript | `CD_init:addr=800a7aec\r\n` (23 bytes) |

BSS clears, heap initialization, original IRQ/VBlank execution and guest CD
callback/state bytes remain verified. Generated guest code and executable data
remain ignored. Diagnostic presentation timing is unchanged.

## Validation and hardware expectations

Host MSVC C++17 /O2 /W4 /WX checks cover data-free, GT2 hash and GT2 boot variants:
MIPS 16/16 (`5775a33b`), GT2 hash 6/6 (`2ceef133`), boot runtime 6/6, IRQ 9/9,
BIOS 16/16, DMA 6/6, GPU 5/5, CD 10/10 and memory control (MC) 5/5.
Generated-fixture checks, emitter tests, portable I/O and geometry tests are included.

MC tests cover every individual bit, upper-bit masking, repeated replacement,
zero and 1325h readback, invalid widths/addresses, aliases, attachment, successful
delay-slot stores, fault preservation and absence of device completions. Boot
watchdogs bracket the COM_DELAY store and the next Timer 1 read; all previous
BIOS/CD/GPU/DICR and unscheduled VBlank checks remain included.

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
parsing pass without compiler warnings. The memory-control object targets
ARMv6K/VFPv2. ELF text/data/BSS: 926,936 / 7,608 / 2,114,336 bytes, excluding
runtime heaps. `3dsxdump`: 69 code / 159 rodata / 2 data / 516 BSS pages.

Copy `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` and its `.smdh` over the
previous SD-card probe. Binary size: 950,592 bytes. SHA-256:
`92dbf9d074990cb7946584f8b80c79c03396ac9f20f1fef3e769d2b3ea1ab78a`.

Expected Old 3DS highlights: `GT2 BOOT PASS (bounded stop)`, PC 8008c35c,
61 entries, 3397094 instructions, `CD 10/10 bank:0 R:1 W:3 MC 5/5`, and boundary
`Timer1 counter (VSync)`. Physical validation of this new checkpoint is pending.
PASS verifies this exact unresolved boundary. CdInit has not completed, no CD
commands execute and no sector data is supplied. Prior text tearing remains
diagnostic UI behavior.
