#include "opengt/boot_probe.hpp"
#include "opengt/startup_services.hpp"
#include <initializer_list>
#include <cstring>

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

    bios = StartupBios{};
    ok = bios.cd_events_open == 0x1F && !bios.cd_dequeue_unresolved;
    for (u32 api : {0x72u, 0x56u}) {
        c = call(0xA0, api, 0);
        const Context before = c;
        ok &= bios.dispatch(c) && (c.sr & 0x401) == 0;
        Context normalized = c; normalized.sr = before.sr;
        ok &= abi_preserved(before, normalized, before.read(2));
        ok &= bios.cd_events_open == 0 && bios.cd_dequeue_unresolved;
    }
    ok &= bios.calls == 2 && bios.cd_remove_calls == 2 && bios.cd_close_attempts == 10 &&
          bios.cd_dequeue_attempts == 2 && bios.critical_entries == 2 && bios.critical_exits == 0;
    check("BIOS_CD_remove_aliases", ok);

    bios = StartupBios{};
    ok = true;
    // Exhaust all six status-stack bits and both external IRQ mask states.
    for (u32 bits = 0; bits < 128; ++bits) {
        const u32 original_sr = 0xA0400200u | (bits & 63) | ((bits & 64) << 4);
        for (u32 api = 1; api <= 2; ++api) {
            c = call(0x80010000, 0, api); c.sr = original_sr;
            c.bad_vaddr = 0x12345678;
            const Context before = c;
            c.begin(); c.fault(Stop::syscall);
            ok &= c.stop == Stop::syscall && c.pc == before.pc && c.epc == before.pc &&
                  c.cause == 0x120 && c.bad_vaddr == before.bad_vaddr;
            ok &= bios.dispatch_syscall(c) && c.stop == Stop::running &&
                  c.pc == before.pc + 4 && c.next_pc == before.pc + 8 &&
                  c.epc == before.pc && c.cause == 0x120 && c.instructions == before.instructions + 1;
            // Current IE and external mask change; previous KU/IE restore;
            // oldest pair now holds the original previous pair after push/RFE.
            const u32 stack = (original_sr & ~0x30u) | ((original_sr & 0xCu) << 2);
            const u32 expected_sr = api == 1 ? (stack & ~0x401u) : (stack | 0x401u);
            ok &= c.sr == expected_sr && c.hi == before.hi && c.lo == before.lo;
            for (unsigned reg = 0; reg < 32; ++reg) {
                const u32 expected = api == 1 && reg == 2 ? ((original_sr & 0x401) == 0x401 ? 1u : 0u) : before.read(reg);
                ok &= c.read(reg) == expected;
            }
        }
    }
    ok &= bios.syscall_calls == 256 && bios.critical_entries == 128 && bios.critical_exits == 128;
    check("BIOS_SYS_status_ABI", ok);

    bios = StartupBios{};
    c = call(0x80010000, 0, 1);
    c.begin(); c.fault(Stop::syscall); ok = bios.dispatch_syscall(c) && c.read(2) == 1;
    c.begin(); c.fault(Stop::syscall); ok &= bios.dispatch_syscall(c) && c.read(2) == 0;
    c.write(4, 2); c.begin(); c.fault(Stop::syscall);
    ok &= bios.dispatch_syscall(c) && (c.sr & 0x401) == 0x401 && c.read(2) == 0;
    check("BIOS_SYS_repeated_enter", ok);

    bios = StartupBios{};
    ok = true;
    for (u32 api : {0u, 3u, 0xFFFFFFFFu}) {
        c = call(0x80010000, 0, api); c.begin(); c.fault(Stop::syscall);
        const Context before = c;
        ok &= !bios.dispatch_syscall(c) && c.stop == Stop::syscall && c.pc == before.pc &&
              c.sr == before.sr && c.cause == before.cause && c.epc == before.epc && c.read(2) == before.read(2);
    }
    c = call(0x80010004, 0, 2); c.next_delay = true;
    c.begin(); c.fault(Stop::syscall);
    ok &= !bios.dispatch_syscall(c) && c.epc == 0x80010000 && c.cause == 0x80000120 &&
          c.stop == Stop::syscall && bios.syscall_calls == 0;
    c = call(0x80010000, 0, 2); ok &= !bios.dispatch_syscall(c);
    check("BIOS_SYS_unknown_BD_guard", ok);

    Memory m = reset_test_memory();
    const char literal[] = "CD_init:%s";
    for (u32 i = 0; i < sizeof(literal); ++i) m.write(0x100 + i, 1, literal[i]);
    bios = StartupBios{};
    ok = true;
    for (u32 vector : {0xA0u, 0xB0u}) {
        c = call(vector, vector == 0xA0 ? 0x3E : 0x3F, vector == 0xA0 ? 0x80000100 : 0xA0000100);
        const Context before = c;
        ok &= bios.dispatch_console(c, m) && abi_preserved(before, c, before.read(2));
    }
    ok &= std::strcmp(bios.console, "CD_init:%sCD_init:%s") == 0 &&
          bios.console_size == 20 && bios.puts_calls == 2 && bios.calls == 2;
    check("BIOS_puts_alias_literal_ABI", ok);

    bios = StartupBios{};
    c = call(0xB0, 0x3F, 0);
    ok = bios.dispatch_console(c, m) && std::strcmp(bios.console, "<NULL>") == 0;
    c = call(0xB0, 0x3F, 0x200); // Mapped empty string.
    ok &= bios.dispatch_console(c, m) && bios.console_size == 6 && bios.puts_calls == 2;
    const char controls[] = "\tX\nY\rZ\b\t!";
    for (u32 i = 0; i < sizeof(controls); ++i) m.write(0x300 + i, 1, controls[i]);
    c = call(0xA0, 0x3E, 0x300);
    ok &= bios.dispatch_console(c, m) &&
          std::strcmp(bios.console, "<NULL>  X\r\nY\rZ\b        !") == 0 && bios.console_column == 9;
    check("BIOS_puts_null_empty_controls", ok);

    bios = StartupBios{};
    // All rejected calls preserve guest state and the existing transcript.
    c = call(0xB0, 0x3F, 0x100); bios.dispatch_console(c, m);
    ok = true;
    for (u32 address : {0x1F801814u, 0xDF800000u, 0xFFFFFFFFu, 0x80000000u}) {
        if (address == 0x80000000u)
            for (u32 i = 0; i <= StartupBios::console_capacity; ++i) m.write(i, 1, 'A');
        c = call(0xB0, 0x3F, address);
        const Context before = c;
        ok &= !bios.dispatch_console(c, m) && c.pc == before.pc && c.read(2) == before.read(2) &&
              c.instructions == before.instructions && c.stop == Stop::running;
    }
    c = call(0xA0, 0x3F, 0x100); ok &= !bios.dispatch_console(c, m); // printf remains unresolved.
    c = call(0xB0, 0x3F, 0x100); c.next_delay = true; ok &= !bios.dispatch_console(c, m);
    c = call(0xB0, 0x3F, 0x100); c.load(4, 0); ok &= !bios.dispatch_console(c, m);
    c = call(0xB0, 0x3F, 0x100); c.stop = Stop::budget; ok &= !bios.dispatch_console(c, m);
    ok &= bios.puts_calls == 1 && bios.calls == 1 && std::strcmp(bios.console, literal) == 0;
    check("BIOS_puts_rejection_atomic", ok);

    bios = StartupBios{};
    for (u32 i = 0; i < StartupBios::console_capacity; ++i) m.write(0x400 + i, 1, 'X');
    m.write(0x500, 1, 0);
    c = call(0xB0, 0x3F, 0x400);
    ok = bios.dispatch_console(c, m) && bios.console_size == 256 && bios.console[256] == 0;
    c = call(0xB0, 0x3F, 0x4FF);
    ok &= !bios.dispatch_console(c, m) && c.pc == 0xB0 && bios.console_size == 256 && bios.calls == 1;
    c = call(0xB0, 0x3F, 0x500);
    ok &= bios.dispatch_console(c, m) && bios.calls == 2; // Empty call fits a full sink.
    bios = StartupBios{};
    for (u32 i = 0; i < 255; ++i) m.write(0x400 + i, 1, 'X');
    m.write(0x4FF, 1, '\n');
    c = call(0xB0, 0x3F, 0x400);
    ok &= !bios.dispatch_console(c, m) && bios.console_size == 0 && bios.calls == 0;
    check("BIOS_puts_capacity_no_truncation", ok);
    return report;
}
} // namespace opengt::guest
