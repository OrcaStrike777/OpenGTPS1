// Generated from tools/generate_mips_tests.py; synthetic instructions only.
#include "opengt/guest.hpp"
namespace opengt::guest::recompiled {
Stop wrap_zero_logic(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 2408FFFF
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0xFFFFFFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 24090001
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 01095021
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 00095823
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, s - t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 2400007B
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, s + 0x0000007Bu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 01096024
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(12, s & t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 01096825
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(13, s | t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 01097026
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(14, s ^ t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 00097827
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(15, ~(s | t));
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop compare_shifts(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 2408FFFF
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0xFFFFFFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 24090001
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 0109502A
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, signed_value(s) < signed_value(t));
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 0109582B
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, s < t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 290C0000
            const u32 s = c.read(8), t = c.read(12);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(12, signed_value(s) < 0LL);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 2D2DFFFF
            const u32 s = c.read(9), t = c.read(13);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(13, s < 0xFFFFFFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 000977C0
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(14, t << 31);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 000E7FC2
            const u32 s = c.read(0), t = c.read(14);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(15, t >> 31);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 000E87C3
            const u32 s = c.read(0), t = c.read(14);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(16, arithmetic_shift(t, 31));
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 01298804
            const u32 s = c.read(9), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(17, t << (s & 31));
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 012E9006
            const u32 s = c.read(9), t = c.read(14);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(18, t >> (s & 31));
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001002Cu: { // MIPS 012E9807
            const u32 s = c.read(9), t = c.read(14);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(19, arithmetic_shift(t, s));
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010030u: { // MIPS 311400FF
            const u32 s = c.read(8), t = c.read(20);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(20, s & 0x000000FFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010034u: { // MIPS 34151234
            const u32 s = c.read(0), t = c.read(21);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(21, s | 0x00001234u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010038u: { // MIPS 3AB6FFFF
            const u32 s = c.read(21), t = c.read(22);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(22, s ^ 0x0000FFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001003Cu: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010040u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop hilo_division(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 2408FFF9
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0xFFFFFFF9u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 24090003
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000003u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 01090018
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            multiply(c, s, t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 00005012
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 00005810
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 0109001A
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            divide(c, s, t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 00006012
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(12, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 00006810
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(13, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 2408FFFF
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0xFFFFFFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 24090002
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000002u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 01090019
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            multiply(c, s, t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001002Cu: { // MIPS 00007010
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(14, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010030u: { // MIPS 00007812
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(15, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010034u: { // MIPS 3C088000
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, 0x80000000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010038u: { // MIPS 2409FFFF
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0xFFFFFFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001003Cu: { // MIPS 0109001A
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            divide(c, s, t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010040u: { // MIPS 00008012
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(16, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010044u: { // MIPS 00008810
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(17, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010048u: { // MIPS 0100001A
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            divide(c, s, t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001004Cu: { // MIPS 00009012
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(18, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010050u: { // MIPS 00009810
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(19, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010054u: { // MIPS 0100001B
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            divide(c, s, t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010058u: { // MIPS 0000A012
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(20, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001005Cu: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010060u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop hilo_moves_unsigned(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 24080007
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000007u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 24090003
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000003u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 0109001B
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            divide(c, s, t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 00005012
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 00005810
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 01000011
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.hi = s;
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 01200013
            const u32 s = c.read(9), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.lo = s;
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 00006010
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(12, c.hi);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 00006812
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(13, c.lo);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 01097020
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            checked_add(c, 14, s, t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 01097822
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            checked_add(c, 15, s, t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001002Cu: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010030u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop memory_endian_sign(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 3C088000
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, 0x80000000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 35080100
            const u32 s = c.read(8), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s | 0x00000100u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 3C0989AB
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, 0x89AB0000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 3529CDEF
            const u32 s = c.read(9), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s | 0x0000CDEFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS AD090000
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            guest_write(c, m, (s + 0x00000000u), 4, t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 810A0000
            const u32 s = c.read(8), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000000u), 1, value)) c.load(10, (value & 0x80u) ? value | 0xFFFFFF00u : value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 910B0000
            const u32 s = c.read(8), t = c.read(11);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000000u), 1, value)) c.load(11, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 850C0002
            const u32 s = c.read(8), t = c.read(12);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000002u), 2, value)) c.load(12, (value & 0x8000u) ? value | 0xFFFF0000u : value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001002Cu: { // MIPS 950D0002
            const u32 s = c.read(8), t = c.read(13);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000002u), 2, value)) c.load(13, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010030u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010034u: { // MIPS 8D0E0000
            const u32 s = c.read(8), t = c.read(14);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000000u), 4, value)) c.load(14, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010038u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001003Cu: { // MIPS A1090004
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            guest_write(c, m, (s + 0x00000004u), 1, t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010040u: { // MIPS A5090006
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            guest_write(c, m, (s + 0x00000006u), 2, t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010044u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010048u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop load_delay(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 24080007
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000007u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 8C080100
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000100u), 4, value)) c.load(8, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 01004821
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 01005021
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 8C080100
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000100u), 4, value)) c.load(8, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 24080009
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000009u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 01005821
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, s + t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 8C000100
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000100u), 4, value)) c.load(0, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop branch_delay(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 24080001
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 11090002
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((s == t) ? 0x80010010u : 0x8001000Cu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 24080002
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000002u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 24090063
            const u32 s = c.read(0), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000063u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 15080002
            const u32 s = c.read(8), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((s != t) ? 0x8001001Cu : 0x80010018u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 240A0003
            const u32 s = c.read(0), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + 0x00000003u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 240B0004
            const u32 s = c.read(0), t = c.read(11);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, s + 0x00000004u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop signed_branches(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 2408FFFF
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0xFFFFFFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 05000002
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((signed_value(s) < 0) ? 0x80010010u : 0x8001000Cu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 25290001
            const u32 s = c.read(9), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 240A0063
            const u32 s = c.read(0), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + 0x00000063u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 05010002
            const u32 s = c.read(8), t = c.read(1);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((signed_value(s) >= 0) ? 0x8001001Cu : 0x80010018u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 25290001
            const u32 s = c.read(9), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 19000002
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((signed_value(s) <= 0) ? 0x80010024u : 0x80010020u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 25290001
            const u32 s = c.read(9), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 240A0063
            const u32 s = c.read(0), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + 0x00000063u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 24080001
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 1D000002
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((signed_value(s) > 0) ? 0x80010034u : 0x80010030u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001002Cu: { // MIPS 25290001
            const u32 s = c.read(9), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010030u: { // MIPS 240A0063
            const u32 s = c.read(0), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + 0x00000063u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010034u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010038u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop call_return_jump(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 03E08021
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(16, s + t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 0C004008
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.write(31, 0x8001000Cu); c.branch(0x80010020u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 24080005
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000005u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 240A0007
            const u32 s = c.read(0), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + 0x00000007u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 0800400C
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
             c.branch(0x80010030u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 240B0008
            const u32 s = c.read(0), t = c.read(11);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, s + 0x00000008u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 25090001
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(9, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010028u: { // MIPS 240C0009
            const u32 s = c.read(0), t = c.read(12);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(12, s + 0x00000009u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001002Cu: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010030u: { // MIPS 02000008
            const u32 s = c.read(16), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010034u: { // MIPS 240D000A
            const u32 s = c.read(0), t = c.read(13);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(13, s + 0x0000000Au);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop jalr_target_capture(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 03E08021
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(16, s + t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 3C088001
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, 0x80010000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 35080020
            const u32 s = c.read(8), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s | 0x00000020u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 0100F809
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.write(31, 0x80010014u); c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 24080000
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010014u: { // MIPS 02000008
            const u32 s = c.read(16), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010018u: { // MIPS 240A0007
            const u32 s = c.read(0), t = c.read(10);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(10, s + 0x00000007u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001001Cu: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010020u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010024u: { // MIPS 240B0008
            const u32 s = c.read(0), t = c.read(11);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(11, s + 0x00000008u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop overflow_trap(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 3C087FFF
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, 0x7FFF0000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 3508FFFF
            const u32 s = c.read(8), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s | 0x0000FFFFu);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 21090001
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            checked_add(c, 9, s, 0x00000001u, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop slot_address_trap(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 10000001
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((s == t) ? 0x80010008u : 0x80010008u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 8C080001
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000001u), 4, value)) c.load(8, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop unmapped_store(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 3C081F80
            const u32 s = c.read(0), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, 0x1F800000u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS AD091000
            const u32 s = c.read(8), t = c.read(9);
            (void)s; (void)t; const u32 next = c.begin();
            guest_write(c, m, (s + 0x00001000u), 4, t);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop bounded_loop(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 1000FFFF
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((s == t) ? 0x80010000u : 0x80010008u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 25080001
            const u32 s = c.read(8), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(8, s + 0x00000001u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}

Stop unknown_target(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 01000008
            const u32 s = c.read(8), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 00000000
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(0, t << 0);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        default: return c.stop = Stop::unknown_pc;
        }
    }
    return c.stop;
}
}

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
    {
        auto& result = report.tests[report.count++];
        result.name = "wrap_zero_logic"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::wrap_zero_logic(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(0), 0x00000000u);
        expect(result, c.read(10), 0x00000000u);
        expect(result, c.read(11), 0xFFFFFFFFu);
        expect(result, c.read(12), 0x00000001u);
        expect(result, c.read(13), 0xFFFFFFFFu);
        expect(result, c.read(14), 0xFFFFFFFEu);
        expect(result, c.read(15), 0xFFFFFFFEu);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "compare_shifts"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::compare_shifts(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(10), 0x00000001u);
        expect(result, c.read(11), 0x00000000u);
        expect(result, c.read(12), 0x00000001u);
        expect(result, c.read(13), 0x00000001u);
        expect(result, c.read(14), 0x80000000u);
        expect(result, c.read(15), 0x00000001u);
        expect(result, c.read(16), 0xFFFFFFFFu);
        expect(result, c.read(17), 0x00000002u);
        expect(result, c.read(18), 0x40000000u);
        expect(result, c.read(19), 0xC0000000u);
        expect(result, c.read(20), 0x000000FFu);
        expect(result, c.read(21), 0x00001234u);
        expect(result, c.read(22), 0x0000EDCBu);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "hilo_division"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::hilo_division(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(10), 0xFFFFFFEBu);
        expect(result, c.read(11), 0xFFFFFFFFu);
        expect(result, c.read(12), 0xFFFFFFFEu);
        expect(result, c.read(13), 0xFFFFFFFFu);
        expect(result, c.read(14), 0x00000001u);
        expect(result, c.read(15), 0xFFFFFFFEu);
        expect(result, c.read(16), 0x80000000u);
        expect(result, c.read(17), 0x00000000u);
        expect(result, c.read(18), 0x00000001u);
        expect(result, c.read(19), 0x80000000u);
        expect(result, c.read(20), 0xFFFFFFFFu);
        expect(result, c.hi, 0x80000000u);
        expect(result, c.lo, 0xFFFFFFFFu);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "hilo_moves_unsigned"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::hilo_moves_unsigned(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(10), 0x00000002u);
        expect(result, c.read(11), 0x00000001u);
        expect(result, c.read(12), 0x00000007u);
        expect(result, c.read(13), 0x00000003u);
        expect(result, c.read(14), 0x0000000Au);
        expect(result, c.read(15), 0x00000004u);
        expect(result, c.hi, 0x00000007u);
        expect(result, c.lo, 0x00000003u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "memory_endian_sign"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::memory_endian_sign(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(10), 0xFFFFFFEFu);
        expect(result, c.read(11), 0x000000EFu);
        expect(result, c.read(12), 0xFFFF89ABu);
        expect(result, c.read(13), 0x000089ABu);
        expect(result, c.read(14), 0x89ABCDEFu);
        { u32 v = 0; expect(result, memory.read(0x00000100u, 4, v), 1); expect(result, v, 0x89ABCDEFu); hash(report, v); }
        { u32 v = 0; expect(result, memory.read(0x00000104u, 1, v), 1); expect(result, v, 0x000000EFu); hash(report, v); }
        { u32 v = 0; expect(result, memory.read(0x00000106u, 2, v), 1); expect(result, v, 0x0000CDEFu); hash(report, v); }
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "load_delay"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        expect(result, memory.write(0x00000100u, 4, 0x0000002Au), 1);
        recompiled::load_delay(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(0), 0x00000000u);
        expect(result, c.read(8), 0x00000009u);
        expect(result, c.read(9), 0x00000007u);
        expect(result, c.read(10), 0x0000002Au);
        expect(result, c.read(11), 0x00000009u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "branch_delay"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        c.write(9, 0x00000001u);
        recompiled::branch_delay(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(8), 0x00000002u);
        expect(result, c.read(9), 0x00000001u);
        expect(result, c.read(10), 0x00000003u);
        expect(result, c.read(11), 0x00000004u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "signed_branches"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::signed_branches(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(9), 0x00000004u);
        expect(result, c.read(10), 0x00000000u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "call_return_jump"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::call_return_jump(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(8), 0x00000005u);
        expect(result, c.read(9), 0x00000006u);
        expect(result, c.read(10), 0x00000007u);
        expect(result, c.read(11), 0x00000008u);
        expect(result, c.read(12), 0x00000009u);
        expect(result, c.read(13), 0x0000000Au);
        expect(result, c.read(31), 0x8001000Cu);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "jalr_target_capture"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::jalr_target_capture(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::returned));
        expect(result, c.read(8), 0x00000000u);
        expect(result, c.read(10), 0x00000007u);
        expect(result, c.read(11), 0x00000008u);
        expect(result, c.read(31), 0x80010014u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "overflow_trap"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::overflow_trap(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::overflow));
        expect(result, c.read(9), 0x00000000u);
        expect(result, c.epc, 0x80010008u);
        expect(result, c.cause, 0x00000030u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "slot_address_trap"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::slot_address_trap(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::address_load));
        expect(result, c.epc, 0x80010000u);
        expect(result, c.cause, 0x80000010u);
        expect(result, c.bad_vaddr, 0x00000001u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "unmapped_store"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::unmapped_store(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::unmapped));
        expect(result, c.bad_vaddr, 0x1F801000u);
        expect(result, c.cause, 0x0000001Cu);
        expect(result, c.epc, 0x80010004u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "bounded_loop"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        recompiled::bounded_loop(c, memory, 32, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::budget));
        expect(result, c.read(8), 0x00000010u);
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
        auto& result = report.tests[report.count++];
        result.name = "unknown_target"; result.passed = true;
        std::memset(ram, 0, sizeof(ram)); std::memset(scratch, 0, sizeof(scratch));
        Context c; c.start(0x80010000u); c.write(31, 0x1FFF0000u);
        c.write(8, 0x80030000u);
        recompiled::unknown_target(c, memory, 256, 0x1FFF0000u);
        expect(result, static_cast<u32>(c.stop), static_cast<u32>(Stop::unknown_pc));
        for (unsigned reg = 0; reg < 32; ++reg) hash(report, c.read(reg));
        hash(report, c.hi); hash(report, c.lo); hash(report, c.pc);
        hash(report, c.cause); hash(report, c.epc); hash(report, c.instructions);
        hash(report, static_cast<u32>(c.stop));
        report.passed += result.passed;
    }
    {
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
