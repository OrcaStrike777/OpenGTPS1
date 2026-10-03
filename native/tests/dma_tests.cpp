#include "opengt/boot_probe.hpp"
#include "opengt/ps1_dma.hpp"

namespace opengt::guest {
TestReport run_dma_tests() noexcept {
    TestReport report{};
    const auto check = [&](const char* name, bool ok) {
        report.tests[report.count++] = {name, ok, ok ? 1u : 0u, 1};
        report.passed += ok;
    };
    constexpr u32 dicr = 0x1F8010F4, stat = 0x1F801070, mask = 0x1F801074;
    InterruptController irq;
    DmaInterrupts dma(irq);
    u32 value = 0;
    bool ok = dma.state() == 0 && dma.read(dicr, 4, value) && value == 0;
    ok &= dma.write(dicr, 4, 0xFFFFFFFF) && dma.state() == 0x80FF807F;
    ok &= dma.irq_rises == 1 && irq.pending() == 8 && !irq.requested();
    dma.reset();
    ok &= dma.state() == 0 && dma.reads == 0 && dma.writes == 0 && dma.irq_rises == 0;
    ok &= irq.pending() == 8; // device reset cannot clear the separate I_STAT latch
    irq.write(stat, 4, ~u32{8});
    ok &= dma.write(dicr, 4, 0x80000000) && dma.state() == 0 && irq.pending() == 0;
    check("DMA_reset_readonly_bits", ok);

    ok = true;
    // All seven channels, all combinations of master/channel enable.
    for (unsigned channel = 0; channel < 7; ++channel) {
        for (unsigned mode = 0; mode < 4; ++mode) {
            dma.reset(); irq.reset();
            const u32 config = ((mode & 1) ? 0x800000u : 0u) |
                               ((mode & 2) ? (1u << (16 + channel)) : 0u);
            dma.write(dicr, 4, config);
            dma.complete(channel);
            const u32 expected = config | (mode == 3 ? (0x80000000u | (1u << (24 + channel))) : 0u);
            ok &= dma.state() == expected && dma.completions == 1;
            ok &= irq.pending() == (mode == 3 ? 8u : 0u);
            if (mode != 3) {
                // Unmasking never fabricates a previously suppressed completion.
                dma.write(dicr, 4, 0xFF0000);
                ok &= dma.state() == 0xFF0000 && irq.pending() == 0;
            }
        }
    }
    dma.complete(7); dma.complete(32);
    ok &= dma.completions == 1;
    check("DMA_completion_gating", ok);

    dma.reset(); irq.reset(); dma.write(dicr, 4, 0xFF0000);
    for (unsigned channel = 0; channel < 7; ++channel) dma.complete(channel);
    ok = dma.state() == 0xFFFF0000 && dma.irq_rises == 1;
    dma.write(dicr, 4, 0x05FF0000); // acknowledge only channels 0 and 2
    ok &= dma.state() == 0xFAFF0000;
    dma.write(dicr, 4, 0x00800000); // channel enables do NOT mask existing flags
    ok &= dma.state() == 0xFA800000 && dma.irq_rises == 1;
    dma.write(dicr, 4, 0); // master off, flags retained
    ok &= dma.state() == 0x7A000000 && irq.pending() == 8;
    irq.write(stat, 4, ~u32{8});
    dma.write(dicr, 4, 0x00800000);
    ok &= dma.state() == 0xFA800000 && irq.pending() == 8 && dma.irq_rises == 2;
    dma.write(dicr, 4, 0x7FFF0000);
    ok &= dma.state() == 0x00FF0000 && irq.pending() == 8; // DICR ack alone is insufficient
    irq.write(stat, 4, ~u32{8});
    ok &= irq.pending() == 0;
    check("DMA_flags_W1C_persistence", ok);

    dma.reset(); irq.reset();
    dma.write(dicr, 4, 0x8000); // force/bus-error bit bypasses every channel mask
    Context c; c.cause = 0x100;
    irq.sync_cpu(c);
    ok = dma.state() == 0x80008000 && irq.pending() == 8 && c.cause == 0x100;
    irq.write(mask, 4, 8); irq.sync_cpu(c); ok &= c.cause == 0x500;
    irq.pulse(7); irq.write(stat, 4, ~u32{8}); irq.sync_cpu(c);
    ok &= irq.pending() == 0x80 && c.cause == 0x100;
    dma.write(dicr, 4, 0x8000); // held DICR line must not continuously re-latch IRQ3
    ok &= irq.pending() == 0x80 && dma.irq_rises == 1;
    dma.write(dicr, 4, 0); dma.write(dicr, 4, 0x8000);
    ok &= irq.pending() == 0x88 && dma.irq_rises == 2;
    dma.reset(); irq.sync_cpu(c);
    ok &= irq.pending() == 0x88 && c.cause == 0x500;
    irq.write(stat, 4, ~u32{8}); irq.sync_cpu(c); ok &= c.cause == 0x100;
    check("DMA_force_IRQ3_ack_edges", ok);

    dma.reset(); irq.reset();
    Memory m = reset_test_memory(); m.attach_dma_interrupts(&dma);
    // Partial stores use the shifted *full* guest register, not RAM byte merging.
    ok = m.write(0xBF8010F4, 2, 0x00FF0041) && dma.state() == 0x00FF0041;
    dma.complete(0); dma.complete(6);
    ok &= m.read(0x9F8010F4, 4, value) && value == 0xC1FF0041;
    ok &= m.read(dicr, 2, value) && value == 0x41;
    ok &= m.read(dicr + 2, 2, value) && value == 0xC1FF;
    ok &= m.read(dicr + 3, 1, value) && value == 0xC1;
    ok &= m.write(dicr + 2, 2, 0x01FF) && dma.state() == 0xC0FF0000;
    ok &= m.write(dicr + 3, 1, 0x40) && dma.state() == 0;
    ok &= dma.reads == 4 && dma.writes == 3;
    ok &= !m.read(dicr + 1, 2, value) && !m.write(dicr + 2, 4, 0);
    ok &= !m.read(0xDF8010F4, 4, value) && !m.write(0x3F8010F4, 4, 0);
    ok &= !m.read(dicr, 0, value) && !m.write(dicr, 8, 0);
    ok &= dma.reads == 4 && dma.writes == 3;
    check("DMA_MMIO_widths_aliases", ok);

    dma.reset(); irq.reset();
    ok = !m.read(0x1F8010A8, 4, value) && !m.write(0x1F8010B8, 4, 0x11000000);
    ok &= dma.channel_reads == 1 && dma.channel_writes == 1 && dma.completions == 0;
    ok &= dma.state() == 0 && irq.pending() == 0;
    m.attach_dma_interrupts(nullptr);
    ok &= !m.read(dicr, 4, value) && !m.write(dicr, 4, 0);
    check("DMA_unmapped_channels", ok);
    return report;
}
} // namespace opengt::guest
