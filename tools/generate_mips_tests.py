#!/usr/bin/env python3
"""Author synthetic MIPS words and independent expected results; emit native tests.

No GT2 instructions/data. Expected registers are hand-specified, not obtained
by executing an interpreter or by evaluating the generated C++.
"""
import argparse
import importlib.util
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
module_spec = importlib.util.spec_from_file_location("emit_cpp", ROOT / "vendor/RecompOne/RecompOne.Recompiler/Native/emit_cpp.py")
backend = importlib.util.module_from_spec(module_spec)
module_spec.loader.exec_module(backend)
BASE = 0x80010000
RETURN = 0x1FFF0000


def r(fn, rd=0, rs=0, rt=0, sa=0):
    return (rs << 21) | (rt << 16) | (rd << 11) | (sa << 6) | fn


def i(op, rt, rs=0, imm=0):
    return (op << 26) | (rs << 21) | (rt << 16) | (imm & 65535)


def j(op, index):
    return (op << 26) | (((BASE + index * 4) >> 2) & 0x3ffffff)


RET = [r(8, rs=31), 0]
CASES = []


def case(name, words, registers=None, stop="returned", initial=None, memory=None,
         expected_memory=None, extra=None, limit=256, returns=True):
    CASES.append(dict(name=name, base=BASE, words=words + (RET if returns else []),
                      registers=registers or {}, stop=stop, initial=initial or {},
                      memory=memory or [], expected_memory=expected_memory or [],
                      extra=extra or {}, limit=limit))


case("wrap_zero_logic", [i(9,8,imm=-1), i(9,9,imm=1), r(33,10,8,9), r(35,11,0,9),
     i(9,0,imm=123), r(36,12,8,9), r(37,13,8,9), r(38,14,8,9), r(39,15,0,9)],
     {0:0, 10:0, 11:0xffffffff, 12:1, 13:0xffffffff, 14:0xfffffffe, 15:0xfffffffe})
case("compare_shifts", [i(9,8,imm=-1), i(9,9,imm=1), r(42,10,8,9), r(43,11,8,9),
     i(10,12,8,0), i(11,13,9,-1), r(0,14,rt=9,sa=31), r(2,15,rt=14,sa=31),
     r(3,16,rt=14,sa=31), r(4,17,9,9), r(6,18,9,14), r(7,19,9,14),
     i(12,20,8,255), i(13,21,imm=0x1234), i(14,22,21,0xffff)],
     {10:1,11:0,12:1,13:1,14:0x80000000,15:1,16:0xffffffff,17:2,
      18:0x40000000,19:0xc0000000,20:255,21:0x1234,22:0xedcb})
case("hilo_division", [i(9,8,imm=-7), i(9,9,imm=3), r(24,rs=8,rt=9), r(18,10),r(16,11),
     r(26,rs=8,rt=9),r(18,12),r(16,13),i(9,8,imm=-1),i(9,9,imm=2),r(25,rs=8,rt=9),
     r(16,14),r(18,15),i(15,8,imm=0x8000),i(9,9,imm=-1),r(26,rs=8,rt=9),
     r(18,16),r(16,17),r(26,rs=8),r(18,18),r(16,19),r(27,rs=8),r(18,20)],
     {10:0xffffffeb,11:0xffffffff,12:0xfffffffe,13:0xffffffff,14:1,15:0xfffffffe,
      16:0x80000000,17:0,18:1,19:0x80000000,20:0xffffffff}, extra={"hi":0x80000000,"lo":0xffffffff})
case("hilo_moves_unsigned", [i(9,8,imm=7),i(9,9,imm=3),r(27,rs=8,rt=9),r(18,10),r(16,11),
     r(17,rs=8),r(19,rs=9),r(16,12),r(18,13),r(32,14,8,9),r(34,15,8,9)],
     {10:2,11:1,12:7,13:3,14:10,15:4},extra={"hi":7,"lo":3})
case("memory_endian_sign", [i(15,8,imm=0x8000),i(13,8,8,0x100), i(15,9,imm=0x89ab),
     i(13,9,9,0xcdef),i(43,9,8),i(32,10,8),0,i(36,11,8),0,i(33,12,8,2),0,
     i(37,13,8,2),0,i(35,14,8),0,i(40,9,8,4),i(41,9,8,6)],
     {10:0xffffffef,11:0xef,12:0xffff89ab,13:0x89ab,14:0x89abcdef},
     expected_memory=[(0x100,4,0x89abcdef),(0x104,1,0xef),(0x106,2,0xcdef)])
case("load_delay", [i(9,8,imm=7),i(35,8,0,0x100),r(33,9,8,0),r(33,10,8,0),
     i(35,8,0,0x100),i(9,8,imm=9),r(33,11,8,0), i(35,0,0,0x100),0],
     {0:0,8:9,9:7,10:42,11:9},memory=[(0x100,4,42)])
# Taken decision must use pre-slot register values. Untaken branches have slots too.
case("branch_delay", [i(9,8,imm=1),i(4,9,8,2),i(9,8,imm=2),i(9,9,imm=99),
     i(5,8,8,2),i(9,10,imm=3),i(9,11,imm=4)],{8:2,9:1,10:3,11:4}, initial={9:1})
case("signed_branches", [i(9,8,imm=-1),i(1,0,8,2),i(9,9,9,1),i(9,10,imm=99),
     i(1,1,8,2),i(9,9,9,1),i(6,0,8,2),i(9,9,9,1),i(9,10,imm=99),
     i(9,8,imm=1),i(7,0,8,2),i(9,9,9,1),i(9,10,imm=99)],{9:4,10:0})
case("call_return_jump", [r(33,16,31,0), j(3,8),i(9,8,imm=5),i(9,10,imm=7),j(2,12),
     i(9,11,imm=8),0,0,i(9,9,8,1),r(8,rs=31),i(9,12,imm=9),0,
     r(8,rs=16),i(9,13,imm=10)],{8:5,9:6,10:7,11:8,12:9,13:10,31:BASE+12},returns=False)
case("jalr_target_capture", [r(33,16,31,0),i(15,8,imm=0x8001),i(13,8,8,32),r(9,31,8),
     i(9,8,imm=0),r(8,rs=16),i(9,10,imm=7),0,r(8,rs=31),i(9,11,imm=8)],
     {8:0,10:7,11:8,31:BASE+20},returns=False)
case("overflow_trap", [i(15,8,imm=0x7fff),i(13,8,8,0xffff),i(8,9,8,1)],
     {9:0},stop="overflow",extra={"epc":BASE+8,"cause":48},returns=False)
case("slot_address_trap", [i(4,0,0,1),i(35,8,0,1),0],stop="address_load",
     extra={"epc":BASE,"cause":0x80000010,"bad_vaddr":1},returns=False)
case("unmapped_store", [i(15,8,imm=0x1f80),i(43,9,8,0x1000)],stop="unmapped",
     extra={"bad_vaddr":0x1f801000,"cause":28,"epc":BASE+4},returns=False)
case("bounded_loop", [i(4,0,0,-1),i(9,8,8,1)],{8:16},stop="budget",limit=32,returns=False)
case("unknown_target", [r(8,rs=8),0],stop="unknown_pc",initial={8:0x80030000},returns=False)


def build_suite():
    source = "// Generated from tools/generate_mips_tests.py; synthetic instructions only.\n"
    source += backend.emit(CASES)
    source += '''
#include "opengt/guest_tests.hpp"
#include <cstring>
namespace opengt::guest {
namespace {
alignas(8) std::uint8_t ram[0x200000];
std::uint8_t scratch[1024];
void expect(TestResult& test, u32 actual, u32 expected) noexcept {
    if (test.passed && actual != expected) {
        test.passed = false; test.actual = actual; test.expected = expected;
    }
}
void hash(TestReport& report, u32 value) noexcept {
    report.signature = (report.signature ^ value) * 16777619u;
}
}
Memory reset_test_memory() noexcept {
    std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
    return Memory(ram, sizeof(ram), scratch);
}
TestReport run_synthetic_tests() noexcept {
    TestReport report{};
    report.signature = 2166136261u;
    Memory memory(ram, sizeof(ram), scratch);
'''
    for spec in CASES:
        source += f'''    {{
        auto& result = report.tests[report.count++];
        result.name = "{spec['name']}"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start({backend.literal(BASE)}); c.write(31, {backend.literal(RETURN)});
'''
        for reg, value in spec["initial"].items(): source += f"        c.write({reg}, {backend.literal(value)});\n"
        for address,width,value in spec["memory"]:
            source += f"        expect(result, memory.write({backend.literal(address)}, {width}, {backend.literal(value)}), 1);\n"
        source += f"        recompiled::{spec['name']}(c, memory, {spec['limit']}, {backend.literal(RETURN)});\n"
        source += f"        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::{spec['stop']}));\n"
        for reg,value in spec["registers"].items(): source += f"        expect(result, c.read({reg}), {backend.literal(value)});\n"
        for member,value in spec["extra"].items(): source += f"        expect(result, c.{member}, {backend.literal(value)});\n"
        for address,width,value in spec["expected_memory"]:
            source += f"        {{ u32 v = 0; expect(result, memory.read({backend.literal(address)}, {width}, v), 1); expect(result, v, {backend.literal(value)}); hash(report, v); }}\n"
        source += '''        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
'''
    source += '''    {
        auto& result = report.tests[report.count++];
        result.name = "memory_bounds_alias"; result.passed = true;
        u32 value = 0;
        expect(result, memory.write(0x80000100, 4, 0x89ABCDEF), 1);
        expect(result, memory.read(0xA0200100, 4, value), 1);
        expect(result, value, 0x89ABCDEF);
        expect(result, memory.read(0x100, 1, value), 1); expect(result, value, 0xEF);
        expect(result, memory.read(0x102, 2, value), 1); expect(result, value, 0x89AB);
        expect(result, memory.write(0x1F8003FC, 4, 0x12345678), 1);
        expect(result, memory.read(0x9F8003FC, 4, value), 1); expect(result, value, 0x12345678);
        expect(result, memory.write(0x1F800400, 1, 7), 0);
        expect(result, memory.write(0x1F8003FF, 4, 7), 0);
        expect(result, memory.read(0x1F8003FC, 4, value), 1); expect(result, value, 0x12345678);
        expect(result, memory.write(0x1FFFFC, 4, 99), 1);
        expect(result, memory.read(0x807FFFFC, 4, value), 1); expect(result, value, 99);
        expect(result, memory.read(0x80800000, 4, value), 0);
        expect(result, memory.read(0xC0000100, 4, value), 0);
        expect(result, memory.read(0xBFC00000, 4, value), 0);
        expect(result, memory.read(0xFFFFFFFF, 4, value), 0);
        Memory invalid(ram, 1024, scratch);
        expect(result, invalid.read(0, 4, value), 0);
        hash(report, result.passed); report.passed += result.passed;
    }
    return report;
}
}
'''
    return source


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    outputs = {
        ROOT / "tests/fixtures/mips/synthetic.json": json.dumps({"functions":CASES}, indent=2) + "\n",
        ROOT / "native/tests/recompiled/mips_suite.cpp": build_suite(),
    }
    for path, text in outputs.items():
        if args.check:
            if not path.is_file() or path.read_text(encoding="utf-8") != text:
                raise SystemExit(f"Stale generated fixture: {path}")
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(text.encode("utf-8"))
    print(f"{'Checked' if args.check else 'Generated'} {len(CASES)} native MIPS functions + memory-boundary test")


if __name__ == "__main__":
    main()
