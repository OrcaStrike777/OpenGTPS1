#include "opengt/boot_probe.hpp"
#include "opengt/ps1_cdrom.hpp"
#include "opengt/ps1_interrupts.hpp"
#include <initializer_list>

namespace opengt::guest {
TestReport run_cdrom_tests() noexcept {
    TestReport report{};
    const auto check = [&](const char* name, bool ok) {
        report.tests[report.count++] = {name, ok, ok ? 1u : 0u, 1};
        report.passed += ok;
    };
    constexpr u32 port = 0x1F801800;
    CdromRegisters cd;
    u32 value = 0;
    check("CD_idle_status", cd.read(port, 1, value) && value == 0x18 &&
          cd.bank() == 0 && cd.reads == 1 && cd.writes == 0);
    bool ok = true;
    for (u32 input = 0; input < 256; ++input) {
        ok &= cd.write(port, 1, input) && cd.bank() == (input & 3);
        ok &= cd.read(port, 1, value) && value == (0x18u | (input & 3));
    }
    check("CD_index_masks_readonly_status", ok && cd.writes == 256 && cd.reads == 257);

    ok = true;
    for (unsigned width : {0u, 2u, 3u, 4u, 8u}) {
        value = 0xDEADBEEF;
        ok &= !cd.write(port, width, 0) && !cd.read(port, width, value);
        ok &= value == 0xDEADBEEF;
    }
    ok &= !cd.write(port - 1, 1, 0) && !cd.read(port + 4, 1, value);
    check("CD_unsupported_width_is_atomic", ok && cd.bank() == 3 && cd.writes == 256 && cd.reads == 257);

    ok = true;
    for (u32 bank = 0; bank < 4; ++bank) {
        ok &= cd.write(port, 1, bank);
        for (u32 offset = 1; offset < 4; ++offset) {
            value = 0xDEADBEEF;
            const bool flags = offset == 3 && (bank & 1);
            if (flags) ok &= cd.read(port + offset, 1, value) && value == 0xE0;
            else ok &= !cd.read(port + offset, 1, value) && value == 0xDEADBEEF;
            // Only low-five-bit HCLRCTL acknowledgements are accepted here.
            for (u32 input = 0; input < 256; ++input) {
                const bool ack = bank == 1 && offset == 3 && input < 32;
                ok &= cd.write(port + offset, 1, input) == ack;
            }
        }
        ok &= cd.bank() == bank && cd.status() == (0x18u | bank);
    }
    check("CD_banked_port_scope", ok && cd.writes == 292 && cd.reads == 259 &&
          cd.flag_reads == 2 && cd.acknowledgements == 32 && cd.interrupt_flags == 0);

    // Seed only the latch, never a completed command, FIFO response or IRQ.
    ok = true;
    for (u32 pending = 0; pending < 32; ++pending) {
        for (u32 ack = 0; ack < 32; ++ack) {
            CdromRegisters latched;
            latched.interrupt_flags = pending;
            ok &= latched.write(port, 1, 3) && latched.read(port + 3, 1, value);
            ok &= value == (0xE0 | pending) && latched.interrupt_flags == pending;
            ok &= latched.write(port, 1, 1) && latched.read(port + 3, 1, value);
            ok &= value == (0xE0 | pending) && latched.write(port + 3, 1, ack);
            ok &= latched.read(port + 3, 1, value) && value == (0xE0 | (pending & ~ack));
            ok &= latched.acknowledgements == 1 && latched.flag_reads == 3;
        }
    }
    check("CD_flags_mirror_selective_W1C", ok);

    CdromRegisters latched;
    latched.interrupt_flags = 0x1B; // INT3 plus both decoder flags.
    latched.write(port, 1, 1);
    ok = latched.write(port + 3, 1, 1) && latched.interrupt_flags == 0x1A;
    for (u32 input = 0x20; input < 256; ++input)
        ok &= !latched.write(port + 3, 1, input) && latched.interrupt_flags == 0x1A;
    for (unsigned width : {0u, 2u, 3u, 4u, 8u}) {
        value = 0xDEADBEEF;
        ok &= !latched.write(port + 3, width, 0x1F) && !latched.read(port + 3, width, value);
        ok &= value == 0xDEADBEEF && latched.interrupt_flags == 0x1A;
    }
    latched.write(port, 1, 3);
    ok &= !latched.write(port + 3, 1, 0x1F) && latched.interrupt_flags == 0x1A;
    latched.write(port, 1, 1);
    ok &= latched.write(port + 3, 1, 0xFFFFFF1F) && latched.interrupt_flags == 0;
    ok &= latched.write(port + 3, 1, 7) && latched.interrupt_flags == 0;
    check("CD_ack_scope_atomic_byte_bus", ok && latched.acknowledgements == 3 && latched.flag_reads == 0);

    Memory m = reset_test_memory();
    ok = !m.write(port, 1, 1) && !m.read(port, 1, value);
    m.attach_cdrom(&cd);
    ok &= m.write(0x9F801800, 1, 1) && m.read(0xBF801800, 1, value) && value == 0x19;
    ok &= m.write(0xBF801800, 1, 2) && m.read(port, 1, value) && value == 0x1A;
    ok &= !m.write(0xDF801800, 1, 0) && !m.read(0x3F801800, 1, value);
    m.write(port, 1, 1);
    ok &= m.read(0xBF801803, 1, value) && value == 0xE0;
    ok &= m.write(0x9F801803, 1, 7) && cd.interrupt_flags == 0;
    m.write(port, 1, 0);
    Context c; c.start(0x8008B80C);
    guest_write(c, m, port + 3, 1, 0);
    ok &= c.stop == Stop::unmapped && c.bad_vaddr == port + 3 &&
          c.epc == 0x8008B80C && c.cause == 0x1C && cd.bank() == 0;
    Context command; command.start(0x80001004); command.in_delay = true;
    m.write(port, 1, 0);
    guest_write(command, m, port + 1, 1, 1);
    ok &= command.stop == Stop::unmapped && command.epc == 0x80001000 &&
          command.cause == 0x8000001C && cd.status() == 0x18;
    check("CD_MMIO_aliases_and_guest_faults", ok);

    InterruptController irq;
    m.attach_interrupts(&irq);
    irq.pulse(2); // Synthetic existing I_STAT latch, independent of HINTSTS.
    cd.interrupt_flags = 3;
    m.write(port, 1, 1);
    ok = m.read(port + 3, 1, value) && value == 0xE3 && cd.interrupt_flags == 3;
    ok &= m.write(port + 3, 1, 7) && cd.interrupt_flags == 0 && irq.pending() == 4;
    ok &= m.read(port + 3, 1, value) && value == 0xE0 && irq.pending() == 4;
    irq.write(0x1F801070, 4, ~u32{4});
    ok &= m.write(port + 3, 1, 0x1F) && irq.pending() == 0 && cd.status() == 0x19;
    check("CD_ack_preserves_separate_ISTAT", ok);
    return report;
}
} // namespace opengt::guest
