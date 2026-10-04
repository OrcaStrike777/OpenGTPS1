// Generated synthetic store-merge fixtures. No game data.
#include "opengt/guest.hpp"
namespace opengt::guest::recompiled {
Stop store_42_0(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS A8880000
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000000u), t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_42_1(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS A8880001
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000001u), t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_42_2(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS A8880002
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000002u), t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_42_3(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS A8880003
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000003u), t, true);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_46_0(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS B8880000
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000000u), t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_46_1(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS B8880001
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000001u), t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_46_2(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS B8880002
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000002u), t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop store_46_3(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS B8880003
            const u32 s = c.read(4), t = c.read(8);
            (void)s; (void)t; const u32 next = c.begin();
            store_merge(c, m, (s + 0x00000003u), t, false);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
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

Stop syscall_resume(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 24020007
            const u32 s = c.read(0), t = c.read(2);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(2, s + 0x00000007u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 24040002
            const u32 s = c.read(0), t = c.read(4);
            (void)s; (void)t; const u32 next = c.begin();
            c.write(4, s + 0x00000002u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 0000000C
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.fault(Stop::syscall);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010010u: { // MIPS 00000000
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

Stop syscall_load(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 8D040000
            const u32 s = c.read(8), t = c.read(4);
            (void)s; (void)t; const u32 next = c.begin();
            u32 value = 0; if (guest_read(c, m, (s + 0x00000000u), 4, value)) c.load(4, value);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 03FFFFCC
            const u32 s = c.read(31), t = c.read(31);
            (void)s; (void)t; const u32 next = c.begin();
            c.fault(Stop::syscall);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010008u: { // MIPS 03E00008
            const u32 s = c.read(31), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch(s);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x8001000Cu: { // MIPS 00000000
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

Stop syscall_slot(Context& c, Memory& m, u32 limit, u32 return_pc) noexcept {
    (void)m;
    while (c.stop == Stop::running) {
        if (c.pc == return_pc && !c.next_delay) return c.stop = Stop::returned;
        if (limit-- == 0) return c.stop = Stop::budget;
        if (c.pc & 3) { c.in_delay = c.next_delay; c.fault(Stop::address_load, c.pc); return c.stop; }
        switch (c.pc) {
        case 0x80010000u: { // MIPS 10000002
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            if (c.in_delay) return c.stop = Stop::delay_control;
            c.branch((s == t) ? 0x8001000Cu : 0x80010008u);
            if (c.stop != Stop::running) return c.stop;
            c.pc = next; break;
        }
        case 0x80010004u: { // MIPS 0000000C
            const u32 s = c.read(0), t = c.read(0);
            (void)s; (void)t; const u32 next = c.begin();
            c.fault(Stop::syscall);
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
        case 0x8001000Cu: { // MIPS 00000000
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

#include "opengt/boot_probe.hpp"
#include "opengt/startup_services.hpp"
namespace opengt::guest {
TestReport run_boot_runtime_tests() noexcept {
    TestReport report{};
    Memory m = reset_test_memory();
    bool ok = true;
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_42_0(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0xDDCCBB11u && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_42_1(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0xDDCC1122u && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_42_2(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0xDD112233u && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_42_3(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0x11223344u && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_46_0(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0x11223344u && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_46_1(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0x223344AAu && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_46_2(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0x3344BBAAu && c.stop==Stop::returned;
    }
    {
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_46_3(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v==0x44CCBBAAu && c.stop==Stop::returned;
    }
    report.tests[report.count++] = {"SWL_SWR_all_offsets",ok,ok,1}; report.passed += ok;
    InterruptController io; u32 value=0;
    ok = !m.read(0x1F801074,2,value);
    m.attach_interrupts(&io);
    ok &= m.read(0x1F801074,2,value) && value==0;
    ok &= m.write(0x9F801074,2,0xFFFF);
    ok &= m.read(0xBF801074,4,value) && value==0x7FF;
    ok &= m.write(0x1F801070,2,0) && !m.read(0x1F801075,2,value);
    ok &= !m.read(0xDF801074,2,value) && !m.read(0x1F801074,1,value);
    ok &= io.mask_reads==2 && io.mask_writes==1 && io.stat_writes==1;
    m.attach_interrupts(nullptr);
    ok &= !m.read(0x1F801074,2,value);
    report.tests[report.count++] = {"I_MASK_opt_in",ok,ok,1}; report.passed += ok;
    {
        StartupBios bios; Context c; c.start(0x80010000); c.write(31,0x1FFF0000);
        recompiled::syscall_resume(c,m,10,0x1FFF0000);
        ok = c.stop==Stop::syscall && c.instructions==3 && c.pc==0x80010008 && c.epc==c.pc;
        ok &= bios.dispatch_syscall(c) && c.pc==0x8001000C && c.read(2)==7 && c.sr==0x401;
        recompiled::syscall_resume(c,m,10,0x1FFF0000);
        ok &= c.stop==Stop::returned && c.instructions==5 && bios.syscall_calls==1;
        report.tests[report.count++] = {"SYS_emitted_resume",ok,ok,1}; report.passed += ok;
    }
    {
        StartupBios bios; Context c; c.start(0x80010000); c.write(31,0x1FFF0000);
        c.write(4,99); c.write(8,0x100); c.sr=0x401; m.write(0x100,4,1);
        recompiled::syscall_load(c,m,10,0x1FFF0000);
        // Exception observes the retired load; the instruction's code field
        // does not select a BIOS API. No pending load survives exception entry.
        ok = c.stop==Stop::syscall && c.pending_register==0 && c.read(4)==1 && c.instructions==2;
        ok &= bios.dispatch_syscall(c) && c.read(2)==1 && (c.sr&0x401)==0;
        recompiled::syscall_load(c,m,10,0x1FFF0000);
        ok &= c.stop==Stop::returned && c.instructions==4;
        report.tests[report.count++] = {"SYS_code_load_retirement",ok,ok,1}; report.passed += ok;
    }
    {
        StartupBios bios; Context c; c.start(0x80010000); c.write(4,2);
        c.cause=0x500; c.bad_vaddr=0xABCDEF00;
        recompiled::syscall_slot(c,m,10,0x1FFF0000);
        ok = c.stop==Stop::syscall && c.pc==0x80010004 && c.epc==0x80010000 &&
             c.cause==0x80000520 && c.bad_vaddr==0xABCDEF00 && c.instructions==2 &&
             !bios.dispatch_syscall(c) && bios.syscall_calls==0;
        report.tests[report.count++] = {"SYS_delay_slot_diagnostic",ok,ok,1}; report.passed += ok;
    }
    return report;
}
}
