# Bounded native GT2 boot probe

This milestone runs the supported US Simulation executable's real entry path,
using offline-generated C++ compiled for ARM11. The previous MIPS and GT2 hash
build passed on an original Old 3DS. **This new boot build needs hardware validation.**

## Reproduce

From the repository root, with the validated IMG/CUE available:

```text
python tools/prepare_native_gt2_boot.py
python tools/test_native_guest.py --boot
```

Run the second command in a Visual Studio compiler prompt (or use `--compiler`
with a C++17 compiler). Generation validates the entire disc and executable
SHA-256, extracts the executable, and generates both hash and boot probes.
`--image PATH` accepts another location for the same supported IMG/CUE pair.
All extracted payloads and generated game code stay under ignored `work/` and
`generated/`; they must not be committed. The local boot binary embeds the
executable payload and is for testing with the user's own dump.

In a devkitPro shell:

```text
make -C platform/3ds WITH_GT2_BOOT=1
```

From Windows PowerShell, if devkitPro tools are not on PATH:

```powershell
& C:/devkitPro/msys2/usr/bin/bash.exe -lc 'cd "$(cygpath -u "$1")" && make -C platform/3ds WITH_GT2_BOOT=1' build-3ds (Get-Location).Path
```

Output: `platform/3ds/opengtps1-old3ds-bootprobe.3dsx`. Copy it to the SD card's
`3ds` directory and launch through Homebrew Launcher. No disc files are required
on the SD card. Default and `WITH_GT2_PROBE=1` builds remain separate targets.
Portable CMake builds can select `-DOPENGT_GT2_BOOT=ON` (also includes the hash).

## Authentic startup and bounds

The PS-X EXE header supplies entry **0x8005D600**, GP=0, load=0x80010000,
payload length=0x99000. Its SP field is zero; SYSTEM.CNF supplies 0x801FFF00.
Executable SHA-256: `4dd40d01a3e83967e2d4301106890eb314d72027802bee077bbbc246f152e331`.

The generator translates these explicit ranges, with exclusive end addresses.
Descriptions are inferred roles from the local instructions, not original symbols.
Unknown transfers stop rather than searching or decoding at runtime.

| Start | End | Observed role |
|---|---|---|
| 8005D600 | 8005D698 | Executable entry/CRT startup |
| 8008CE08 | 8008CE30 | Startup memory-clear helper |
| 8008DDB4 | 8008DE24 | Heap preparation |
| 8008DD74 | 8008DDB4 | Heap initialization |
| 80010998 | 800109B0 | Initialization wrapper |
| 8005D9BC | 8005D9F0 | Clear wrapper |
| 8008CE30 | 8008CEDC | Memory-set loop |
| 8008BC78 | 8008BCA8 | Indirect library-table call |
| 8008BE0C | 8008BE64 | Interrupt initialization |

The original memory-clearing loops, direct calls, table load/indirect call,
returns and delay slots execute unchanged. A poisoned BSS range
0x800A8D5C..0x801F0D60 proves the guest performs its clearing; the resulting heap
pointer at 0x800A8D50 is checked against 0x801F0D60. General registers start zero
apart from the executable GP and stack. This is a deterministic loader fixture,
not an emulation of the BIOS loader's entire machine state.

New integer operations are little-endian SWL/SWR, needed to translate the
memory-set routine's unaligned paths. Supplemental generated tests cover both
instructions at all four byte offsets. Existing tests and signatures are unchanged.

Without any shim, the first hardware access stops at guest PC 0x8008BE44,
I_MASK address 0x1F801074. The only enabled shim is a deterministic I_MASK latch,
initially zero, permitting 16/32-bit reads/writes, masking writes to 11 bits,
and handling physical/KSEG0/KSEG1 aliases. It is opt-in on each Memory object.
After two I_MASK accesses, startup reaches **I_STAT (0x1F801070)** at
**PC 0x8008BE50** and stops with `unmapped memory`. No interrupt scheduling,
I_STAT semantics, BIOS services, GPU, CD, SPU, GTE, overlays or callbacks are
implemented. The next step is to validate this checkpoint on Old 3DS, then
inspect that I_STAT access and implement only the interrupt behavior needed
for the next authentic startup checkpoint.

Execution has a hard 2,000,000-instruction watchdog; a separate PC test proves
an explicit 32-instruction budget stops at exactly 32. The graph is iterative,
with no host recursion, guest instruction decoding, or dynamically growing trace.
It reuses the test suite's 2 MiB RAM and 1 KiB scratchpad, plus a 626,688-byte
constant executable image and 16-entry trace. Initialization and verification
loops have fixed bounds. This is a correctness probe, not cycle-accurate timing.

## Expected results and screen

PC tests pass with warnings treated as errors, including the no-shim boundary
and watchdog checks. The ARM build contains the same synthetic, hash, supplemental
and boot code. On launch (and X rerun), expect:

```text
MIPS 16/16 hash 5775a33b
GT2 hash 6/6 sig 2ceef133
Boot runtime 2/2
GT2 BOOT PASS (bounded stop)
Entry       8005d600
PC          8008be50
Last OK     8008be4c
Last func   8008be0c
Functions   9
Instructions 1303078
Unresolved  1f801070
Trap: unmapped memory
I_STAT boundary: reached
BSS clear: PASS / I_MASK IO: 2
```

The nine traced entries follow the table order above. Function count means
entries visited (including CRT entry), not completed functions. Last OK is the
last successfully executed instruction. Instruction count includes the faulting
access. PASS requires the expected boundary, nine entries, BSS clearing and heap
pointer checks; it does **not** indicate a completed game boot.

Y toggles the synthetic-details page, X reruns all tests and the boot probe,
A changes the top-screen clear color while held, and START exits. No game image,
menus, audio or racing appears. A brief pause during the bounded startup run is
expected; host VBlank timing is not used as PS1 timing.
