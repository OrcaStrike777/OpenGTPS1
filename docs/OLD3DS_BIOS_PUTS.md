# Native BIOS puts and the next CdInit console call

This continues `429a755` on `old3ds-port`. The user validated that commit on
physical Old 3DS: all diagnostics passed, reaching 53 entries / 3,396,889
instructions at BIOS B(3Fh). Its push to `origin/old3ds-port` is confirmed.

The user also observed transient bottom-screen text tearing: a moving vertical
strip of characters and a diagonal redraw boundary. There was no crash, failed
test or abnormal guest boot state. This is recorded as diagnostic UI behavior;
there is no evidence here connecting it to PS1 execution. Presentation timing
is unchanged by this checkpoint.

The new checkpoint handles the original BIOS `puts`, retains its complete output
`CD_init:`, and returns to the guest. Eight further original instructions reach
**BIOS A(3Fh), printf**, at **000000A0**. That is the next authentic blocker and
remains unresolved. No guest code, console call or hardware access is skipped.

## Console scope

`StartupBios::dispatch_console` handles A(3Eh)/B(3Fh) puts through a 256-byte
transcript shared by host and ARM11 diagnostics. It reads a NUL-terminated string
from mapped guest RAM/scratch, preserves literal percent characters, handles the
null-pointer string, and expands tabs and linefeeds for TTY output. It does not
append a host-C `puts` newline or invent a return-register value.

A complete call is staged before committing its output. Invalid input,
unterminated strings and insufficient transcript space leave the transcript and
guest context unchanged and the call unresolved. The sink never silently drops
bytes. MMIO input is rejected before reading a device. The buffer and staging
area have fixed size and require no allocation. This is a bounded diagnostic
TTY sink, not a general BIOS file-descriptor/console-driver implementation.

The host prints the complete transcript as hex; the 3DS page shows a printable
preview and byte/call counts while the complete bytes remain in `BootReport`.
Control characters cannot become native console escape sequences in the preview.

The HLE transition consumes one watchdog step and returns to guest RA, without
adding an executed guest instruction or advancing the provisional video clock.
This follows the existing BIOS service timing policy; BIOS execution latency is
not cycle accurate. GPU, IRQ and four-frame wait behavior are unchanged.

Reference: [PSX-SPX BIOS TTY console](https://psx-spx.consoledev.net/ps1/kernelbios/tty-console-std-io/).

## Observed boundary

| Observation | Value |
|---|---|
| Guest entries / instructions | 54 / 3,396,897 |
| PC / last successful PC / last entry | 000000A0 / 8008DFFC / 8008DFF4 |
| Stop / API | unresolved BIOS / A(3Fh), printf |
| Format pointer / first argument / RA | 80090EA0 / 800A7AEC / 8008B704 |
| Completed puts calls / transcript bytes | 1 / 8 |
| Transcript hex | 43445F696E69743A |
| BIOS calls handled / SYS calls | 9 / 1 |
| Scheduler edges / half-cycle phase | 6 / 504 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| Temporary callback / in-interrupt flag | 0 / 0 |
| GP1 writes / status reads / status | 4 / 0 / 14802000 |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 15/5 |
| I_STAT / I_MASK / SR | 000 / 009 / 00000401 |
| Cause / EPC | 00000000 / 80010974 |

BSS clears and heap initialization remain verified. The sparse translation
extends 8008B6D8..8008B6F0 to 8008B6D8..8008B704 and adds the original
8008DFF4..8008E000 printf wrapper (end-exclusive ranges). All instructions come
from the locally hash-validated executable. Guest data and generated code remain
ignored. A(3Fh) is distinguished from B(3Fh), rather than acknowledging all APIs
with the same function number.

## Validation and hardware test

Reproduce using the existing compiler environment:

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py
python tools/test_native_guest.py --gt2-probe
python tools/test_native_guest.py --boot
make -B -C platform/3ds WITH_GT2_BOOT=1
```

MSVC C++17 /O2 /W4 /WX checks passed: MIPS 16/16 (`5775a33b`), GT2 hash 6/6
(`2ceef133`), boot runtime 6/6, IRQ 9/9, BIOS 12/12, DMA 6/6, GPU 5/5.
The emitter/generation checks and portable I/O/geometry tests also passed.
New BIOS tests cover aliases and ABI, literal formatting characters, empty/null
strings, TTY controls, invalid inputs and atomic capacity rejection. Integration
tests bracket puts entry/return and verify captured output, unchanged video
phase across the HLE call, and the exact next printf boundary.

devkitPro dependency check and forced data-free, GT2 hash and GT2 boot builds
passed without compiler warnings. The boot object uses ARMv6K/VFPv2;
`3dsxdump` accepted 62 code / 158 rodata / 2 data / 516 BSS pages.
ELF text/data/BSS are 898,448 / 7,608 / 2,114,336 bytes (excluding runtime heaps).

The new `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` is 921,984 bytes.
SHA-256: `00599f3701b2f38df6d7b22553fad5ff507710f8c5ca2f1f53cee5d5bd518350`.
Copy it and its `.smdh` over the previous SD-card probe. Expected highlights:

```text
BIOS 12/12 DMA 6/6 / handled: 9
GT2 BOOT PASS (bounded stop)
PC          000000a0
Last OK     8008dffc
Guest entries 54
Instructions 3396897
VBL count:4/4 SDK count:4
Boundary: BIOS A(3f) printf
TTY [CD_init:] puts:1 bytes:8
```

PASS means the exact expected unresolved boundary, not completed CdInit or a
playable game. Physical validation of this new checkpoint is pending. The next
task begins with A(3Fh) printf; no formatted output or CD hardware behavior has
been fabricated here.
