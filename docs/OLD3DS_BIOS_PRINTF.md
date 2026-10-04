# Native BIOS printf and the CD-ROM index boundary

This continues physically validated `008b3a3` on `old3ds-port`. The user reports
all diagnostics passing at 54 entries / 3,396,897 instructions, stopped at
BIOS A(3Fh) printf. That commit has been pushed to `origin/old3ds-port`.

The new checkpoint executes the console call, returns to original guest code,
and reaches a **byte write of 1 to 1F801800**, the CD-ROM index register, at
**8008B754**. No CD register access is accepted or discarded. This is the next
authentic blocker, before any CD command or response is implemented.

## Implemented console behavior

The original format at 80090EA0 is `addr=%08x\n`, with A1=800A7AEC. The new
bounded A(3Fh) handler parses the format from guest memory and formats the
argument, adding `addr=800a7aec\r\n` to the existing transcript. The complete
transcript is 23 bytes: `CD_init:addr=800a7aec\r\n`.

The supported subset is literal text, `%%`, `%x`, `%X`, and fixed minimum field
width with space or zero padding (including `%08x`). Width never truncates a
value; hexadecimal arguments are 32-bit unsigned words. Arguments come from
A1/A2/A3 and then guest stack words at SP+10h, SP+14h, etc. Guest format strings
are never passed to a host varargs function, and guest pointers are never used
as host pointers. Other conversions, flags, precision, length modifiers and a
null format remain unresolved without changing the guest or transcript.

The prior puts behavior and TTY expansion remain shared. Output is staged in
a fixed buffer and committed only when the entire call succeeds. Unreadable or
unterminated formats, MMIO inputs, invalid stack reads, excessive widths and
full output buffers reject the whole call without truncation or partial output.
Format scanning and output are both bounded at 256 bytes. No allocation or
general-purpose libc formatter is introduced.

The BIOS API reference does not specify a printf result register; the inspected
OpenBIOS implementation also supplies no explicit return value. This bounded
service preserves V0, as with puts, rather than inventing a host-libc byte count.
GT2 does not use a printf result before the next blocker. The service returns
to RA and consumes one watchdog step, with no invented guest instructions.
The existing provisional one-cycle-per-instruction video timing is unchanged;
BIOS service latency remains unmodeled.

References inspected:

- [PSX-SPX BIOS TTY specification](https://psx-spx.consoledev.net/ps1/kernelbios/tty-console-std-io/)
- [Pinned OpenBIOS stdio implementation](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/fileio/stdio.c)

## Authentic progress

The translated range 8008B6D8..8008B704 is extended to 8008B6D8..8008B758
(end exclusive), adding 21 original instructions. These clear CD callback
state, call the existing ResetCallback path, register the CD IRQ handler through
the original SDK callback code, and attempt the first CD register write. No
guest branches, calls, polling loops or memory writes are replaced.

The original SDK stores handler 8008BA2C at 800A7B88 and changes I_MASK from
009 to 00D, enabling IRQ2 while retaining IRQ0/IRQ3. No CD IRQ is fabricated.
Execution stops on the actual `SB` to 1F801800 with value 1.

| Observation | Value |
|---|---|
| Guest entries / attempted instructions | 58 / 3,397,012 |
| PC / last successful PC / last entry | 8008B754 / 8008B750 / 8008C0B4 |
| Stop / unmapped address / write | unmapped memory / 1F801800 / byte 01 |
| Cause / EPC | 0000001C / 8008B754 (not a delay slot) |
| BIOS calls handled / puts / printf / SYS | 10 / 1 / 1 / 1 |
| Transcript bytes / hex | 23 / 43445F696E69743A616464723D38303061376165630D0A |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 16/7 |
| I_STAT / I_MASK / SR | 000 / 00D / 00000401 |
| CD callback slot / handler | 800A7B88 / 8008BA2C |
| Scheduler edges / half-cycle phase | 6 / 734 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| Temporary VBlank callback / in-interrupt flag | 0 / 0 |
| GP1 writes / status reads / status | 4 / 0 / 14802000 |
| DMA completions / DMA IRQ rises | 0 / 0 |

BSS clears and heap initialization remain verified. The earlier no-device
I_MASK stop, unscheduled VBlank wait, BIOS/DICR checkpoints and watchdogs remain
covered. Generated guest code and executable data stay ignored.

## Validation and hardware expectations

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py
python tools/test_native_guest.py --gt2-probe
python tools/test_native_guest.py --boot
make -B -C platform/3ds WITH_GT2_BOOT=1
```

Host MSVC C++17 /O2 /W4 /WX checks passed: MIPS 16/16 (`5775a33b`), GT2 hash
6/6 (`2ceef133`), boot runtime 6/6, IRQ 9/9, BIOS 16/16, DMA 6/6 and GPU 5/5.
Generated fixtures, emitter tests, portable I/O and geometry checks passed.
Printf tests cover ABI, zero/upper-bit values, width/padding, literal percent,
register/stack arguments, invalid formats and atomic rejection at capacity.
Boot checks bracket printf entry/return and the CD store; the expected stop
also verifies exact transcript bytes and the guest's registered CD handler.

devkitPro dependency check, forced builds of all three variants (data-free,
GT2 hash, GT2 boot), and boot 3DSX parsing passed. No compiler warnings occurred.
ELF text/data/BSS: 907,032 / 7,608 / 2,114,336 bytes, excluding runtime heaps.
`3dsxdump`: 64 code / 158 rodata / 2 data / 516 BSS pages.

Copy `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` and its `.smdh` over the
previous SD-card probe. The binary is 930,624 bytes; SHA-256:
`84fefe6c6ed0a086e9950fd4f01994e2b7e0d5ccfb7cf3e78cf5a7ff607a630e`.

Expected highlights: BIOS 16/16, `GT2 BOOT PASS (bounded stop)`, PC 8008b754,
58 entries, 3397012 instructions, IRQ mask 00d, and `CD-ROM index select`.
The compact TTY line shows `CD_init:addr=800a7aec..` with `P:1 F:1`; the two
dots are the sanitized CR/LF preview. The full bytes remain in BootReport and
the host hex log. The shorter footer keeps this longer output within the native
text row; presentation timing is unchanged. Previously reported transient
tearing remains diagnostic UI behavior without evidence of guest-state failure.

Physical validation of this new checkpoint is pending. PASS verifies the exact
unresolved boundary, not completed CdInit, CD emulation or a playable game.
