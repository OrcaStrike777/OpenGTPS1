# Native MIPS execution milestone

Status, 2026-10-02: the user confirmed the native MIPS tests (16/16,
`5775a33b`) and real GT2 hash tests (6/6, `2ceef133`) pass on an original Old 3DS.
The next build adds a [bounded real startup probe](OLD3DS_BOOT_PROBE.md),
which still needs its own hardware run. No game graphics/audio is implemented.

## Runtime and generation

`native/runtime/guest.cpp` and `native/include/opengt/guest.hpp` implement 32
GPRs with hardwired zero, HI/LO, PC/next-PC, pending-load and branch-slot state,
instruction counts and explicit stop reasons. Minimal COP0 storage supplies
SR, Cause, EPC, BadVAddr and PRId for diagnostics. Overflow/alignment stops
record the instruction and branch-delay flag; exception-vector execution and
coprocessor instructions are not implemented.

Memory is caller-owned. Explicit 2 MiB retail or 8 MiB devkit RAM is accepted;
tests share **one 2 MiB RAM arena plus 1 KiB scratchpad**, with no heap allocation.
Aligned 8/16/32-bit accesses are little endian, support KSEG0/KSEG1 aliases and
retail RAM mirrors across the first 8 MiB, and bound the scratchpad. Accesses
validate before writing; KSEG2, unimplemented BIOS/MMIO and invalid accesses
fail instead of silently mapping to RAM. Integer helpers avoid host signed
overflow and unaligned pointer accesses.

`vendor/RecompOne/RecompOne.Recompiler/Native/emit_cpp.py` adapts RecompOne's
instruction fields and emission logic into an offline native C++ path. It accepts
explicit bounded function ranges; it does not yet integrate the full managed
discovery/overlay/patch pipeline. It preserves the existing C# backend. Python
is only a development dependency, and no CLR is needed on-device.

Instructions are decoded at generation time. Output contains direct C++ integer
operations and a guest-PC switch, not runtime opcode decoding, interpretation
or JIT. Both taken and untaken branches execute a slot; decisions, targets and
links are captured before that slot. Operands are captured before pending-load
retirement. Unknown destinations, instruction-budget exhaustion and control
transfers inside a branch slot terminate explicitly. This first implementation
prioritizes deterministic behavior over basic-block optimization.

Supported subset: NOP; SLL/SRL/SRA and variable forms; ADD/ADDU/SUB/SUBU,
ADDI/ADDIU; AND/OR/XOR/NOR and immediate forms; LUI; SLT/SLTU/SLTI/SLTIU;
MULT/MULTU/DIV/DIVU; MFHI/MFLO/MTHI/MTLO; LB/LBU/LH/LHU/LW, SB/SH/SW;
BEQ/BNE/BLEZ/BGTZ/BLTZ/BGEZ; J/JAL/JR/JALR. Divide-by-zero and signed
division overflow are defined without host undefined behavior.

Still unsupported: unaligned merge instructions, coprocessor operations/GTE,
REGIMM link branches, BIOS/devices, interrupts, cache behavior, self-modifying
code, overlay replacement and exception handlers. The budget is not PS1 cycle
accounting; the load-delay fixtures are not exhaustive pipeline verification.
Architecture reference: [IDT R30xx manual](https://usermanual.wiki/Document/r3000manual.723589236.pdf).

## Synthetic tests

`tools/generate_mips_tests.py` authors synthetic words and independent expected
values, producing committed JSON and C++ fixtures. They contain no game bytes.
The same generated functions and checks run on PC and in both 3DS builds.

| Test | Checks |
| --- | --- |
| wrap_zero_logic | Wraparound, subtraction, zero register, bitwise operations |
| compare_shifts | Signed/unsigned comparisons, immediates, fixed/variable shifts |
| hilo_division | Products, negative division, overflow, zero divisors |
| hilo_moves_unsigned | HI/LO transfers, unsigned quotient/remainder, arithmetic |
| memory_endian_sign | Endianness, signed/unsigned loads, byte/halfword/word stores |
| load_delay | Old/new load values and intervening register overwrite |
| branch_delay | Taken/untaken slots; operand changed after branch decision |
| signed_branches | BLTZ/BGEZ/BLEZ/BGTZ conditions and slots |
| call_return_jump | J/JAL/JR, link address and return slots |
| jalr_target_capture | Slot overwrites target register after indirect call decision |
| overflow_trap | ADDI overflow preserves destination |
| slot_address_trap | Unaligned LW in slot, EPC/Cause.BD/bad address |
| unmapped_store | Device access fails rather than writing ordinary RAM |
| bounded_loop | Infinite loop stops at instruction budget |
| unknown_target | Uncompiled destination stops explicitly |
| memory_bounds_alias | Mirrors, aliases, scratch end, invalid regions/sizes |

PC result: **16/16, signature `5775A33B`**. The signature folds guest state,
selected memory results, stops and instruction counts. Six Python rejection-test
groups pass for unsupported/undefined encodings and malformed input. Trap tests
pass when their expected diagnostic stop occurs, not when they return normally.

In a Visual Studio developer prompt (or with `--compiler c++`):

```sh
python tools/generate_mips_tests.py --check
python tools/test_native_guest.py
```

Regenerate intentionally without `--check` after fixture/backend changes.
The runner checks freshness and compiles with optimization and warnings as errors.
MSVC 19.16 `/O2 /W4 /WX` passed both suites. The standalone `native/portable`
CMake project also contains the guest targets; that CMake path is unverified.

## Exact GT2 probe

The user's raw Simulation image passed full validation:

```text
Disc bytes: 691850208
Disc SHA: D0AB6E70539601057590A36299543C0ADAD219254D712F7D4273219094ED5031
EXE: SCUS_944.88 (628736 bytes)
EXE SHA: 4DD40D01A3E83967E2D4301106890EB314D72027802BEE077BBBC246F152E331
Entry: 0x80083004; 44 bytes through the delay slot at 0x8008302C
Range SHA: AC1136F4EBBE0581665179024172282817B82325C4B5F4D8DF8A428FE28B1284
```

OpenGTPS1's `tools/prepare_reference.py` already identifies this string-hash
entry and continuation at `0x80083008`. Its algorithm starts at zero, rotates
left six bits then adds each unsigned byte, and stops at NUL. The same backend
emits its exact 11 instructions, not a host replacement for the hash algorithm.
An independent high-level formula supplies expected results only:

| Input | Expected V0 |
| --- | --- |
| Empty | `00000000` |
| `A` | `00000041` |
| `GT2` | `00048532` |
| `Gran Turismo 2` | `9D2306F7` |
| Bytes `80 FF 41` | `00084001` |
| 128 repetitions of `A` | `4D555555` |

All six pass natively on PC, including pointer advancement, unchanged input,
return completion and unchanged HI/LO. Probe signature: **`2CEEF133`**.
The 3DS calls this probe only after its own synthetic suite passes. Neither
suite in the new build has yet been observed on hardware.

```sh
python tools/prepare_native_gt2_probe.py
python tools/test_native_guest.py --gt2-probe
```

Preparation reuses `tools/psx_iso.py`, validates CUE association, disc/executable
hashes and range, and writes only ignored `work/` and `generated/old3ds/` outputs.
Game instructions, extracted executable and generated GT2 code are not committed.
No external developer checkout is required by this extraction path.

## 3DS builds and next hardware check

In devkitPro MSYS2, from the repository root:

```sh
make -C platform/3ds
make -C platform/3ds WITH_GT2_PROBE=1
```

Default remains data-free. The optional build requires local generation and uses
a separate object directory/filename. Both build with devkitARM r68/GCC 16.1.0,
libctru 2.7.0, Citro3D 1.7.1, without New 3DS modes. Both passed packaging/SMDH
checks; `3dsxdump` parsed the probe. Its ELF text/data/BSS are
161528/7608/2114336 bytes, including the shared 2 MiB guest arena.

For the next hardware test, copy these local files to `sdmc:/3ds/opengtps1/`:

```text
platform/3ds/opengtps1-old3ds-gt2probe.3dsx
platform/3ds/opengtps1-old3ds-gt2probe.smdh
```

Probe 3DSX: 184808 bytes, SHA-256
`142FCC430010605409B6570B88C57A33F84FFBA8105D4D323BA83D24E0EBA446`.
Synthetic-only `platform/3ds/opengtps1-old3ds.3dsx`: 181524 bytes.
Do not copy the disc or extracted EXE to SD for this test; neither is used there.

The launcher title is **OpenGTPS1 GT2 hash probe** (the data-free build is
**OpenGTPS1 MIPS tests**). Expected: all 16 rows show **PASS**, summary
`MIPS 16/16 hash 5775a33b`, and
`GT2 hash 6/6 sig 2ceef133`. X reruns both suites; START exits. Existing input,
timing and A color diagnostics remain. Missing optional `bootstrap.txt` is
harmless. A synthetic failure shows the first actual/expected mismatch and gates
the GT2 call. Zero GT2 cases means the probe was not built or was gated.

Next: confirm these new results on Old 3DS, then extend coverage based on the
next small GT2 function's actual requirements. This leaf probe is not game boot.
