# Native CD-ROM Request disable and COM_DELAY boundary

This continues physically validated `034720d`, pushed to `origin/old3ds-port`.
The user reports that its Old 3DS validation passed without issue. `main` remains
at d9e6af5. The new checkpoint accepts the original byte write of zero to
1F801803 in bank 0 at 8008B80C, then stops at a **32-bit write of 00001325 to
1F801020 (COM_DELAY)** at **PC 8008B820**. This is in the delay slot of the
original JAL at 8008B81C, so **EPC is 8008B81C and Cause is 8000001C**.

## Implemented behavior and scope

HCHPCTL, also called the CD-ROM Request register, is written at 1F801803 in
bank 0. Its active controls are BFRD (buffer read), BFWR (buffer write) and SMEN
(manual XA sound-map enable). Writing zero leaves these disabled. The bounded
controller has no active transfer, queued sector or data FIFO contents, so this
write preserves its idle state and leaves DRQ clear. A diagnostic counter records
the accepted write. It does not acknowledge interrupt flags or change I_STAT.

Only the zero bus byte is supported, including full register values whose low
byte is zero. Nonzero values, including reserved bits, reject atomically. Wider
accesses also remain unsupported. These are runtime scope limits, not claims
that hardware rejects such accesses. Supporting transfer enables later requires
the corresponding real buffer/transfer behavior; this checkpoint supplies no
sector bytes, responses, completed commands or IRQs. Disable during a populated
or active transfer is outside the current empty-controller model.

Bank-0 reads at the same address still target HINTMSK and remain unresolved;
Request has no invented readback. The bank-1 flag acknowledgement and bank-1/3
flag reads retain their prior behavior. Other banked writes remain unresolved.

References checked:

- [Sony CXD1199AQ host-control register definitions, page 35](https://www.alldatasheetpt.com/html-pdf/46932/SONY/CXD1199AQ/3075/35/CXD1199AQ.html)
- [PSX-SPX CD-ROM register source and initialization sequence](https://github.com/psx-spx/psx-spx.github.io/blob/master/docs/cdromdrive.md)
- [PSX-SPX common bus timing register](https://psx-spx.consoledev.net/ps1/system/memorycontrol/)

## Authentic execution

The translated range 8008B6D8..8008B810 extends to 8008B6D8..8008B824
(end exclusive), adding five instructions from the validated original EXE:

| PC | Operation |
|---|---|
| 8008B80C | Existing SB writes 00 to bank-0 Request successfully |
| 8008B810 | LUI builds the SDK register-pointer table address |
| 8008B814 | LW loads 1F801020 from guest RAM at 800A7AD8 |
| 8008B818 | ADDIU loads 00001325 into V0 and retires the LW delay |
| 8008B81C | JAL schedules target 8008B034 and writes RA=8008B824 |
| 8008B820 | Delay-slot SW attempts the COM_DELAY write and traps |

The call target has not executed because its delay-slot store is unresolved.
No branch, call, delay slot or guest instruction is bypassed. COM_DELAY is a
memory-control timing register, not another CD register bank; it is left
unimplemented at this checkpoint.

| Observation | Value |
|---|---|
| Guest entries / attempted instructions | 58 / 3,397,042 |
| PC / last successful PC / last entry | 8008B820 / 8008B81C / 8008C0B4 |
| Unmapped access | 1F801020, 32-bit write 00001325 |
| Cause / EPC / RA | 8000001C / 8008B81C / 8008B824 |
| CD reads / writes / Request writes / bank / status | 1 / 3 / 1 / 0 / 18 |
| CD flag reads / acknowledgement writes / pending flags | 1 / 0 / 00 |
| I_STAT reads/writes / I_MASK reads/writes | 12/5 / 16/7 |
| I_STAT / I_MASK / SR | 000 / 00D / 00000401 |
| Scheduler edges / half-cycle phase | 6 / 794 |
| IRQ entries / returns / active | 4 / 4 / no |
| SDK VBlank count / GT2 wait count | 4 / 4 |
| GP1 writes / reads / status | 4 / 0 / 14802000 |
| BIOS calls / puts / printf / SYS | 10 / 1 / 1 / 1 |
| Console transcript | `CD_init:addr=800a7aec\r\n` (23 bytes) |

BSS clears, heap initialization, original IRQ/VBlank execution and guest CD
callback/state bytes remain verified. Video timing and diagnostic presentation
are unchanged. Generated guest code and executable data remain ignored.

## Validation and hardware expectations

Host MSVC C++17 /O2 /W4 /WX validation covers data-free, GT2 hash and GT2 boot
variants: MIPS 16/16 (`5775a33b`), GT2 hash 6/6 (`2ceef133`), boot runtime 6/6,
IRQ 9/9, BIOS 16/16, DMA 6/6, GPU 5/5 and CD 10/10. Generated-fixture checks,
emitter tests, portable I/O and geometry tests are included.

Request tests check repeated zero writes, low-byte truncation, all nonzero byte
values, unsupported widths/segments, physical and cached/uncached aliases,
bank isolation, unchanged CD flags/I_STAT, clear DRQ and unavailable sector,
response and command ports. Boot watchdogs bracket Request disable, the JAL and
its delay slot, checking counts, PCs, RA, fault Cause and EPC. Existing index,
flags, BIOS, GPU, DICR and unscheduled VBlank checks remain included.

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
ELF text/data/BSS: 916,592 / 7,608 / 2,114,336 bytes, excluding runtime heaps.
`3dsxdump`: 66 code / 158 rodata / 2 data / 516 BSS pages.

Copy `platform/3ds/opengtps1-old3ds-bootprobe.3dsx` and its `.smdh` over the
previous SD-card probe. Binary size: 940,220 bytes. SHA-256:
`7182768cbc51315ecb9c69ec4ed2d8532b4de9063ecc94cb242008e6c5996c22`.

Expected Old 3DS highlights: `GT2 BOOT PASS (bounded stop)`, PC 8008b820,
58 entries, 3397042 instructions, `CD 10/10 bank:0 R:1 W:3`, and boundary
`COM_DELAY (bus timing)`. Physical validation of this new checkpoint is pending.
PASS confirms this exact unresolved boundary. CdInit has not completed, no CD
commands execute and no sector data is supplied. Prior text tearing remains
diagnostic UI behavior.
