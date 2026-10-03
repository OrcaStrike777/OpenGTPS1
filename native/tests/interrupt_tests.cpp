#include "opengt/boot_probe.hpp"
#include "opengt/startup_services.hpp"

namespace opengt::guest {
TestReport run_interrupt_tests() noexcept {
    TestReport report{};
    const auto check = [&](const char* name, bool ok) {
        report.tests[report.count++] = {name, ok, ok ? 1u : 0u, 1};
        report.passed += ok;
    };
    InterruptController irq;
    u32 value = 0;
    bool ok = irq.pending() == 0 && irq.mask() == 0 && !irq.requested();
    for (unsigned bit = 0; bit <= 10; ++bit) irq.pulse(bit);
    irq.pulse(11); irq.pulse(32);
    ok &= irq.read(0x1F801070, 4, value) && value == 0x7FF;
    ok &= !irq.requested(); // requests latch while every source is masked
    irq.reset();
    ok &= irq.pending() == 0 && irq.mask() == 0 && irq.stat_reads == 0;
    check("IRQ_reset_pending", ok);

    irq.set_line(2, true); irq.set_line(9, true);
    ok = irq.write(0x1F801070, 2, ~u32{1u << 2}) && irq.pending() == (1u << 9);
    irq.set_line(2, true); // same asserted level cannot re-latch an acknowledged edge
    ok &= irq.pending() == (1u << 9);
    irq.set_line(2, false); irq.set_line(2, true);
    ok &= irq.pending() == ((1u << 2) | (1u << 9));
    ok &= irq.write(0x1F801070, 4, 0xFFFFFFFF) && irq.pending() == 0x204;
    ok &= irq.write(0x1F801070, 4, 4) && irq.pending() == 4;
    ok &= irq.write(0x1F801070, 2, 0) && irq.pending() == 0;
    ok &= irq.write(0x1F801070, 4, 0xFFFFFFFF) && irq.pending() == 0; // cannot create requests
    check("IRQ_ack_edges", ok);

    irq.reset(); irq.pulse(10);
    Context c; c.cause = 0x80000324; // preserve software IRQs, exception code and BD
    irq.sync_cpu(c);
    ok = c.cause == 0x80000324;
    ok &= irq.write(0x1F801074, 2, 0x400) && irq.requested();
    irq.sync_cpu(c); ok &= c.cause == 0x80000724;
    ok &= irq.write(0x1F801074, 4, 0xFFFFFFFF) && irq.mask() == 0x7FF;
    ok &= irq.write(0x1F801074, 4, 0) && irq.pending() == 0x400 && !irq.requested();
    irq.sync_cpu(c); ok &= c.cause == 0x80000324;
    ok &= irq.write(0x1F801074, 4, 0x400) && irq.requested();
    ok &= irq.write(0x1F801070, 4, 0) && !irq.requested();
    irq.sync_cpu(c); ok &= c.cause == 0x80000324;
    check("IRQ_mask_cpu_signal", ok);

    Memory m = reset_test_memory(); irq.reset();
    ok = !m.read(0x1F801070, 2, value) && !m.write(0x1F801074, 4, 1);
    m.attach_interrupts(&irq); irq.pulse(8);
    ok &= m.read(0x9F801070, 2, value) && value == 0x100;
    ok &= m.read(0xBF801070, 4, value) && value == 0x100;
    ok &= m.write(0xBF801074, 2, 0xFFFF);
    ok &= m.read(0x1F801074, 4, value) && value == 0x7FF;
    ok &= m.write(0x9F801070, 4, 0xFFFFFEFF) && irq.pending() == 0;
    irq.pulse(10);
    ok &= m.read(0x1F801072, 2, value) && value == 0;
    ok &= m.read(0x1F801076, 2, value) && value == 0;
    ok &= m.write(0x1F801072, 2, 0) && irq.pending() == 0x400;
    ok &= m.write(0x1F801076, 2, 0) && irq.mask() == 0x7FF;
    const auto reads = irq.stat_reads + irq.mask_reads, writes = irq.stat_writes + irq.mask_writes;
    ok &= !m.read(0x1F801071, 2, value) && !m.write(0x1F801070, 1, 0);
    ok &= !m.read(0xDF801070, 4, value) && !m.write(0x3F801074, 2, 0);
    ok &= !m.read(0x1F801078, 4, value) && !m.write(0x1F801072, 4, 0);
    ok &= reads == 5 && writes == 4 && reads == irq.stat_reads + irq.mask_reads &&
          writes == irq.stat_writes + irq.mask_writes;
    m.attach_interrupts(nullptr);
    ok &= !m.read(0x1F801070, 4, value);
    check("IRQ_MMIO_width_alias", ok);

    DmaPriority dma; TimerSetup timers; StartupBios bios;
    m.attach_dma_priority(&dma); m.attach_timer_setup(&timers);
    ok = m.read(0x1F8010F0, 4, value) && value == 0x07654321;
    ok &= m.write(0x1F8010F0, 4, 0x33333333) && dma.value == 0x33333333;
    ok &= m.write(0x9F8010F2, 2, 0x7654) && dma.value == 0x76543333;
    ok &= m.write(0xBF8010F0, 2, 0x3210) && dma.value == 0x76543210;
    ok &= m.read(0x1F8010F2, 2, value) && value == 0x7654;
    ok &= !m.write(0x1F8010F4, 4, 0); // DICR is an independent unresolved device
    timers.counter[1] = 17;
    ok &= m.write(0x1F801114, 4, 0x100) && timers.mode[1] == 0x100 && timers.counter[1] == 0;
    ok &= m.write(0xBF801104, 2, 0xFFFF) && timers.mode[0] == 0x3FF;
    ok &= !m.read(0x1F801114, 4, value) && !m.write(0x1F801110, 4, 0);
    c = Context{}; c.start(0xB0); c.write(9, 0x19); c.write(4, 0x80001038);
    c.write(31, 0x80010000); c.write(2, 0x123);
    ok &= bios.dispatch(c) && bios.hook_buffer == 0x80001038 &&
          bios.interrupt_environment == 0x80001002 && bios.calls == 1 &&
          c.pc == 0x80010000 && c.next_pc == 0x80010004 && c.read(2) == 0x123;
    c.start(0xB0); c.write(9, 0x5C);
    ok &= !bios.dispatch(c) && c.pc == 0xB0 && bios.calls == 1;
    check("IRQ_startup_helpers", ok);
    return report;
}
} // namespace opengt::guest
