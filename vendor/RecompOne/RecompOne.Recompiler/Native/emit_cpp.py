#!/usr/bin/env python3
"""Experimental bounded native backend for RecompOne's R3000 instruction model.

Offline word decoding follows Disasm/Instruction.cs and CodeGen/InstructionEmitter.cs.
This entry point intentionally needs only Python, not RecompOne.Runtime or Rabbitizer.
Input: explicit function ranges as little-endian instruction words in JSON.
No runtime instruction decoding, dynamic allocation, or managed runtime is emitted.
"""
import argparse
import json
from pathlib import Path
import re


def number(value):
    return int(value, 0) if isinstance(value, str) else int(value)


def literal(value):
    return f"0x{value & 0xffffffff:08X}u"


def operation(word, pc):
    op, rs, rt, rd, sa, fn = word >> 26, (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, (word >> 6) & 31, word & 63
    imm = word & 65535
    signed = imm if imm < 32768 else imm - 65536
    address = f"(s + {literal(signed)})"
    put = lambda dst, expr: f"c.write({dst}, {expr});"
    branch = lambda cond: (f"c.branch(({cond}) ? {literal(pc + 4 + signed * 4)} : {literal(pc + 8)});", True)
    if op == 0:
        invalid = ((fn in (0, 2, 3) and rs != 0) or
                   (fn in (4, 6, 7, 32, 33, 34, 35, 36, 37, 38, 39, 42, 43) and sa != 0) or
                   (fn == 8 and (rt != 0 or rd != 0 or sa != 0)) or
                   (fn == 9 and (rt != 0 or sa != 0 or rs == rd)) or
                   (fn in (16,18) and (rs != 0 or rt != 0 or sa != 0)) or
                   (fn in (17,19) and (rt != 0 or rd != 0 or sa != 0)) or
                   (fn in (24,25,26,27) and (rd != 0 or sa != 0)))
        if invalid: raise ValueError(f"reserved/undefined encoding {word:08X} at {pc:08X}")
        expressions = {
            0: f"t << {sa}", 2: f"t >> {sa}", 3: f"arithmetic_shift(t, {sa})",
            4: "t << (s & 31)", 6: "t >> (s & 31)", 7: "arithmetic_shift(t, s)",
            16: "c.hi", 18: "c.lo", 33: "s + t", 35: "s - t",
            36: "s & t", 37: "s | t", 38: "s ^ t", 39: "~(s | t)",
            42: "signed_value(s) < signed_value(t)", 43: "s < t",
        }
        if fn in expressions: return put(rd, expressions[fn]), False
        # The 20-bit instruction code is not the PS1 BIOS selector (a0 is).
        # Trap even in a delay slot so EPC/BD remain diagnostic; BIOS policy
        # decides which syscalls can resume without an exception interpreter.
        if fn == 12: return "c.fault(Stop::syscall);", False
        if fn in (32, 34): return f"checked_add(c, {rd}, s, t, {'true' if fn == 34 else 'false'});", False
        if fn in (17, 19): return f"c.{'hi' if fn == 17 else 'lo'} = s;", False
        if fn in (24, 25, 26, 27):
            return f"{'multiply' if fn < 26 else 'divide'}(c, s, t, {'true' if fn % 2 == 0 else 'false'});", False
        if fn == 8: return "c.branch(s);", True
        if fn == 9 and rs != rd: return put(rd, literal(pc + 8)) + " c.branch(s);", True
    if op in (2, 3):
        target = ((pc + 4) & 0xf0000000) | ((word & 0x3ffffff) << 2)
        return (put(31, literal(pc + 8)) if op == 3 else "") + f" c.branch({literal(target)});", True
    if op == 1 and rt in (0, 1): return branch("signed_value(s) " + ("< 0" if rt == 0 else ">= 0"))
    if op == 4: return branch("s == t")
    if op == 5: return branch("s != t")
    if op == 6 and rt == 0: return branch("signed_value(s) <= 0")
    if op == 7 and rt == 0: return branch("signed_value(s) > 0")
    immediate = {9: f"s + {literal(signed)}", 10: f"signed_value(s) < {signed}LL",
                 11: f"s < {literal(signed)}", 12: f"s & {literal(imm)}",
                 13: f"s | {literal(imm)}", 14: f"s ^ {literal(imm)}", 15: literal(imm << 16)}
    if op in immediate and (op != 15 or rs == 0): return put(rt, immediate[op]), False
    if op == 8: return f"checked_add(c, {rt}, s, {literal(signed)}, false);", False
    if op in (32, 33, 35, 36, 37):
        width = {32: 1, 33: 2, 35: 4, 36: 1, 37: 2}[op]
        expr = "value"
        if op == 32: expr = "(value & 0x80u) ? value | 0xFFFFFF00u : value"
        if op == 33: expr = "(value & 0x8000u) ? value | 0xFFFF0000u : value"
        return f"u32 value = 0; if (guest_read(c, m, {address}, {width}, value)) c.load({rt}, {expr});", False
    if op in (42, 46):
        return f"store_merge(c, m, {address}, t, {'true' if op == 42 else 'false'});", False
    if op in (40, 41, 43):
        width = {40: 1, 41: 2, 43: 4}[op]
        return f"guest_write(c, m, {address}, {width}, t);", False
    raise ValueError(f"unsupported instruction {word:08X} at {pc:08X}")


def emit_function(spec):
    name = spec["name"]
    if not re.fullmatch(r"[a-z][a-z0-9_]*", name): raise ValueError("invalid function name")
    base = number(spec["base"])
    words = [number(w) for w in spec["words"]]
    if base < 0 or base & 3 or not words or len(words) > 4096 or base + 4 * len(words) > 0x100000000:
        raise ValueError("invalid/oversized function range")
    if any(word < 0 or word > 0xffffffff for word in words):
        raise ValueError("instruction word is not uint32")
    lines = [f"Stop {name}(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {{",
             "    (void)m;", "    while (c.stop == Stop::running) {",
             "        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;",
             "        if (limit-- == 0) return c.stop = Stop::budget;",
             "        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }",
             "        switch (c.pc) {"]
    for i, word in enumerate(words):
        pc = base + i * 4
        statement, control = operation(word, pc)
        lines += [f"        case {literal(pc)}: {{ // MIPS {word:08X}",
                  f"            const u32 s = c.read({(word >> 21) & 31}), t = c.read({(word >> 16) & 31});",
                  "            (void)s; (void)t; const u32 next = c.begin();"]
        if control:
            lines += ["            if (c.in_delay) return c.stop = Stop::delay_control;"]
        lines += ["            " + statement, "            if (c.stop != Stop::running) return c.stop;",
                  "            c.pc = next; break;", "        }"]
    lines += ["        default: return c.stop = Stop::unknown_pc;", "        }", "    }",
              "    return c.stop;", "}"]
    return "\n".join(lines)


def emit(specs):
    names = [s["name"] for s in specs]
    if len(names) != len(set(names)): raise ValueError("duplicate function name")
    return '#include "opengt/guest.hpp"\nnamespace opengt::guest::recompiled {\n' + \
        "\n\n".join(emit_function(spec) for spec in specs) + "\n}\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    result = emit(json.loads(args.manifest.read_text())["functions"])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(result.encode("utf-8"))


if __name__ == "__main__":
    main()
