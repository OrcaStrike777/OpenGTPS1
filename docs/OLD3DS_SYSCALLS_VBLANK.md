# Native BIOS cleanup/syscalls and the GT2 VBlank wait

Historical checkpoint at `88e4fc2`. The current build enters the original guest
IRQ handler and callback; see [VBlank IRQ delivery](OLD3DS_VBLANK_IRQ.md).

This continues `7c0d7b2` on `old3ds-port`. The real, hash-validated Simulation
executable now passes `_96_remove`, executes `ExitCriticalSection`, returns from
`ResetCallback`, and registers its VBlank callback. The next authentic blocker
is **PS1 VBlank scheduling and interrupt/callback delivery**, before `CdInit`.
This checkpoint passes the host suites and builds for original 3DS/2DS ARM11.
Testing this new binary on physical Old 3DS hardware is still pending.

## Implemented behavior and limits

A(72h) and its A(56h) alias model the bounded BIOS CD lifecycle: enter critical
state, close the five ACK/DNE/RDY/END/ERR events, and record the handler-dequeue
attempt. The loader fixture starts with those five event flags open; it is not
a general event table and does not expose fabricated event handles. Repeated
calls still record five close attempts each. The call does not exit critical
state, clear I_STAT, alter I_MASK/DICR, or issue a CD command. Its void result is
not fabricated in V0. Handler removal remains explicitly unresolved because
retail `SysDeqIntRP` is bugged; the model does not claim successful dequeue or
reproduce an unspecified corrupt linked-list traversal.

The native emitter recognizes MIPS SYSCALL, regardless of its 20-bit code field.
It retires a preceding load and records exception code 8, EPC, BD, and the
R3000 status-stack push. Pending interrupt bits and BadVAddr are preserved.
An opt-in startup BIOS dispatcher handles only SYS(01h) and SYS(02h), selected
by A0. Enter returns whether both incoming current IE and external interrupt
mask were enabled; Exit preserves V0. Both apply the BIOS saved-status change
to bits 2/10, then perform the RFE status-stack pop and resume at EPC+4.
Other guest registers and HI/LO are preserved. EPC retains the trapping PC in
this bounded HLE projection; it does not execute a retail exception handler.

Unknown APIs and delay-slot syscalls remain stopped with exception diagnostics.
The native guest core alone does not automatically handle BIOS syscalls. Each
handled BIOS transition consumes one watchdog step; the SYSCALL instruction
also consumes its own instruction step. Internal operations of the modeled
A(72) service do not invent guest instruction counts or separate SYS calls.

References inspected (no reference implementation code copied):

- [OpenBIOS CD lifecycle at b8a9080](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/cdrom/cdrom.c), `deinitCDRom`.
- [OpenBIOS syscall handler at b8a9080](https://github.com/grumpycoders/pcsx-redux/blob/b8a9080da96576a3ec2cc26114e459381d845324/src/mips/openbios/handlers/syscall.c).
- [PSX-SPX interrupt/exception handling](https://psx-spx.consoledev.net/ps1/kernelbios/interrupt-exception-handling/), critical-section ABI and the dequeue bug.
- [PSX-SPX BIOS CD-ROM functions](https://psx-spx.consoledev.net/ps1/kernelbios/cdrom-functions/), A(56)/A(72) aliases and loader initialization.

## Authentic execution and next work

The disc SHA-256 and SCUS_944.88 executable SHA-256 were both revalidated against
the existing supported revision. No guest instructions were replaced. These
additional ranges were inspected locally and translated (ends exclusive):

| Range | Role |
|---|---|
| 8008C948..8008C958 | Original ExitCriticalSection wrapper |
| 800109B0..800109C0 | Continue system initialization through the VSync setup call |
| 80010954..80010998 | Register VBlank callback, wait for four callbacks, unregister |
| 8008BD08..8008BD3C | Original SDK callback-registration wrapper |
| 8008C60C..8008C638 | Original indexed callback-table setter |

The intermediate checkpoint is **800109B0 / 24 entries / 1,307,611 instructions**.
This is the original return to system initialization, not the final blocker.
GT2 then stores callback **80010928** in **800A8C54** (callback-table slot 4) and
polls its original RAM counter **80011DF4** in **8001096C..8001097C** until it is
at least four. The inspected callback increments that counter after a call to
8007F830. Neither callback is force-invoked by the probe.

At the default **2,000,000-step** watchdog limit:

| Observation | Value |
|---|---|
| Stop / PC / last successful PC | budget / 80010974 / 80010970 |
| Guest entries / attempted instructions | 27 / 1,999,995 |
| Most recently entered routine | 8008C60C |
| Counter / polls | 0 of 4 / 138,471 |
| BIOS calls / SYS calls | 4 / 1 |
| Critical entries / exits | 1 / 1 |
| SR / Cause / EPC | 00000401 / 00000020 / 8008C94C |
| CD remove / close attempts / dequeue attempts | 1 / 5 / 1 |
| CD event flags / dequeue status | 0 / unresolved |
| I_STAT reads/writes / I_MASK reads/writes | 0/1 / 3/5 |
| Pending IRQs / IRQ mask | 000 / 009 |
| DICR reads/writes / value | 0/1 / 00000000 |
| DMA completions / IRQ edges / channel accesses | 0 / 0 / 0 |

The five non-instruction steps are the four BIOS vector services and one SYS
dispatch. Interrupt enables are now set correctly; the counter stays zero
because the runtime has no PS1 VBlank source or CPU exception/callback delivery.
The next implementation needs deterministic PS1 timing, IRQ0 delivery through
the registered guest interrupt environment, execution of the original handler
and callback graph, acknowledgement, and return-from-exception. It must honor
the PAD/VBlank auto-ack policies already disabled by GT2. A host frame wait or
manual counter increment would not validate that path. `CdInit` at 80089F38 is
still unentered; no CD transfer, GPU rendering, audio, or game menu is implied.

## Validation and reproduction

```text
python tools/prepare_native_gt2_boot.py --image "PATH/TO/SUPPORTED.img"
python tools/test_native_guest.py --boot
python tools/test_native_guest.py
python tools/test_native_guest.py --gt2-probe
make -C platform/3ds WITH_GT2_BOOT=1
```

Run host tests in a compiler environment, passing `--compiler` with the full
C++ compiler path if needed. All three configurations pass MSVC C++17 /O2 /W4
/WX. Results: MIPS **16/16** (`5775a33b`), optional GT2 hash **6/6** (`2ceef133`),
boot runtime **5/5**, IRQ **5/5**, BIOS **8/8**, DMA **6/6**; emitter rejection
tests and generated-fixture checks pass. New tests exhaust status-stack bits,
both external-mask states, Enter/Exit register preservation, repeated Enter,
CD aliases/repeated cleanup, unknown APIs, emitted syscall resume, load
retirement, nonzero code fields, and delay-slot EPC/BD.

Integration checks preserve every earlier I_STAT/DICR/BIOS budget checkpoint,
then check CD entry/return, syscall instruction/trap/dispatch, ResetCallback
return, callback registration, and a full unchanged counter-poll iteration.
The final PASS predicate requires the exact persistent wait and unchanged
DMA/IRQ state, BSS clearing, and heap initialization. PASS means this expected
bounded stop; it does not mean completed boot or successful VBlank delivery.

Output: `platform/3ds/opengtps1-old3ds-bootprobe.3dsx`. The binary embeds the
user's validated executable; no disc files are needed on the SD card. Generated
game code, extracted data and binaries remain ignored and uncommitted.

The devkitARM build completed without warnings. Binary size: **874,608 bytes**.
SHA-256: `7ebd146893b48aad617e2c454d4c891065a09a14c7cdb93a66464db98b11cca3`.
devkitPro `3dsxdump` accepted the container: 52 code / 157 rodata / 2 data /
516 BSS pages.

The bottom screen identifies `Old 3DS VBlank probe`, shows `GT2 BOOT PASS`, PC
`80010974`, 27 guest entries, 1,999,995 instructions, and `VBL count:0/4` with
138,471 polls. `deq:?` means handler dequeue remains unresolved. The boundary
line reads `VBlank IRQ/callback wait` and the stop is `budget`. X reruns,
Y shows synthetic details, A changes the top-screen color, START exits.
