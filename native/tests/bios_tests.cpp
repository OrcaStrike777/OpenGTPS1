#include "opengt/boot_probe.hpp"
#include "opengt/startup_services.hpp"

namespace opengt::guest {
namespace {
Context call(u32 vector, u32 api, u32 a0, u32 a1 = 0) noexcept {
    Context c;
    for (unsigned i = 1; i < 32; ++i) c.write(i, 0xAABB0000u + i);
    c.start(vector); c.write(9, api); c.write(4, a0); c.write(5, a1);
    c.write(31, 0x80012340);
    c.hi = 0x98765432; c.lo = 0x12345678; c.sr = 0x401;
    c.cause = 0x100; c.epc = 0x80002000; c.instructions = 17;
    return c;
}
bool abi_preserved(const Context& before, const Context& after, u32 result) noexcept {
    bool ok = after.read(2) == result && after.pc == before.read(31) &&
              after.next_pc == after.pc + 4 && !after.next_delay && !after.in_delay &&
              after.stop == Stop::running && after.instructions == before.instructions &&
              after.hi == before.hi && after.lo == before.lo && after.sr == before.sr &&
              after.cause == before.cause && after.epc == before.epc;
    for (unsigned i = 0; i < 32; ++i)
        if (i != 2) ok &= after.read(i) == before.read(i);
    return ok;
}
}
TestReport run_bios_tests() noexcept {
    TestReport report{};
    const auto check = [&](const char* name, bool ok) {
        report.tests[report.count++] = {name, ok, ok ? 1u : 0u, 1};
        report.passed += ok;
    };
    StartupBios bios;
    bool ok = bios.pad_auto_ack == 0 && bios.calls == 0;
    const u32 flags[] = {1, 0, 0x80000000, 0xFFFFFFFF, 0};
    u32 old = 0;
    for (u32 flag : flags) {
        Context c = call(0xB0, 0x5B, flag), before = c;
        ok &= bios.dispatch(c) && abi_preserved(before, c, old) && bios.pad_auto_ack == flag;
        old = flag;
    }
    ok &= bios.calls == 5 && bios.pad_calls == 5 && bios.rcnt_calls == 0;
    check("BIOS_PAD_flag_ABI", ok);

    InterruptController irq;
    irq.pulse(0); irq.pulse(7); irq.pulse(3);
    irq.write(0x1F801074, 4, 0x89);
    ok = !bios.acknowledge_pad_vblank(irq) && irq.pending() == 0x89;
    Context c = call(0xB0, 0x5B, 1);
    ok &= bios.dispatch(c) && irq.pending() == 0x89 && irq.stat_writes == 0;
    ok &= bios.acknowledge_pad_vblank(irq) && irq.pending() == 0x88 && irq.mask() == 0x89;
    irq.pulse(0);
    c = call(0xB0, 0x5B, 0);
    ok &= bios.dispatch(c) && !bios.acknowledge_pad_vblank(irq) && irq.pending() == 0x89;
    ok &= irq.stat_reads == 0 && irq.stat_writes == 1 && irq.mask_reads == 0 && irq.mask_writes == 1;
    check("BIOS_PAD_VBlank_ack", ok);

    bios = StartupBios{};
    ok = bios.pad_auto_ack == 0;
    for (unsigned timer = 0; timer < 4; ++timer) {
        ok &= bios.timer_auto_ack[timer] == 1;
        old = 1;
        for (u32 flag : flags) {
            c = call(0xC0, 0x0A, timer, flag);
            const Context before = c;
            ok &= bios.dispatch(c) && abi_preserved(before, c, old) && bios.timer_auto_ack[timer] == flag;
            old = flag;
        }
        for (unsigned next = timer + 1; next < 4; ++next) ok &= bios.timer_auto_ack[next] == 1;
    }
    ok &= bios.rcnt_calls == 20 && bios.calls == 20 && bios.pad_calls == 0;
    check("BIOS_RCnt_flag_ABI", ok);

    bios = StartupBios{};
    ok = bios.calls == 0 && bios.pad_calls == 0 && bios.rcnt_calls == 0;
    // Invalid timer indices are unresolved, never host out-of-bounds accesses.
    const u32 invalid[] = {4, 0xFFFFFFFF};
    for (u32 timer : invalid) {
        c = call(0xC0, 0x0A, timer);
        ok &= !bios.dispatch(c) && c.pc == 0xC0 && c.read(2) == 0xAABB0002;
    }
    c = call(0xA0, 0x5B, 1); ok &= !bios.dispatch(c);
    c = call(0xB0, 0x5C, 1); ok &= !bios.dispatch(c);
    c = call(0xB0, 0x5B, 1); c.next_delay = true; ok &= !bios.dispatch(c);
    c = call(0xB0, 0x5B, 1); c.load(4, 0); ok &= !bios.dispatch(c);
    c = call(0xB0, 0x5B, 1); c.stop = Stop::budget; ok &= !bios.dispatch(c);
    ok &= bios.calls == 0 && bios.pad_auto_ack == 0 && bios.timer_auto_ack[3] == 1;
    check("BIOS_bounds_reset_guard", ok);
    return report;
}
} // namespace opengt::guest
