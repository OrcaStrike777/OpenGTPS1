# Experimental native C++ backend

`emit_cpp.py` is an offline native output path adapted from the word fields and
instruction operations in RecompOne's `Disasm/Instruction.cs` and
`CodeGen/InstructionEmitter.cs`. It avoids the managed runtime dependency for
bounded functions. It does not yet run RecompOne's full discovery/overlay/patch
pipeline or replace the existing C# backend.

```sh
python vendor/RecompOne/RecompOne.Recompiler/Native/emit_cpp.py manifest.json output.cpp
```

The JSON shape is `{"functions": [{"name": "example", "base": "0x80010000",
"words": ["0x03E00008", "0x00000000"]}]}`. Words represent little-endian
instruction values, not a byte string. Each bounded range is at most 4096 words.
Names must be lowercase C++ identifiers. Unsupported instructions, malformed
ranges, and duplicate symbols fail generation rather than emitting no-ops.

Each output function contains C++ operations selected by guest PC, with branch
destinations known at generation time. There is no runtime opcode decoder,
instruction fetch, JIT, or interpreter. The PC switch deliberately favors
observable correctness over basic-block optimization in this first milestone.
Indirect transfers can target only addresses included in that function's range;
unknown targets and exhausted instruction budgets terminate explicitly.

The runtime ABI is `opengt/guest.hpp`. Registers are integer guest values, not
host pointers. Source operands are captured before the previous pending load
retires. Both taken and untaken branches execute one delay-slot instruction.
Branch decisions, indirect targets, and link addresses precede slot execution.
Control transfers inside a branch slot stop as unsupported/unpredictable.

Supported integer subset: SLL/SRL/SRA, SLLV/SRLV/SRAV, ADD/ADDU/SUB/SUBU,
ADDI/ADDIU, AND/OR/XOR/NOR, ANDI/ORI/XORI, LUI, SLT/SLTU/SLTI/SLTIU,
MULT/MULTU/DIV/DIVU, MFHI/MFLO/MTHI/MTLO, LB/LBU/LH/LHU/LW, SB/SH/SW,
BEQ/BNE/BLEZ/BGTZ/BLTZ/BGEZ, J/JAL/JR/JALR and NOP. JALR with identical
source and link registers is rejected. The runtime models arithmetic and
alignment diagnostic traps, not exception-vector execution.

Not implemented: LWL/LWR/SWL/SWR, COP instructions/GTE, link variants of REGIMM
branches, interrupts, cache isolation, MMIO, BIOS, relocation/overlay dispatch,
self-modifying code, or guest exceptions returning through vectors. Minimal
COP0 storage supplies SR, Cause, EPC, BadVAddr and PRId for diagnostics; it is
not a working privileged coprocessor. This is not yet a full GT2 execution engine.

Synthetic inputs and generated C++ are reproducible and committed. Actual game
function output is produced only from the user's validated disc and stays under
ignored `generated/old3ds/`. See `docs/OLD3DS_NATIVE_GUEST.md` at repository root
for test commands, memory bounds, hardware checks and the real GT2 hash probe.
