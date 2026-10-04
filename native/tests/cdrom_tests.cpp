#include "opengt/boot_probe.hpp"
#include "opengt/ps1_cdrom.hpp"
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
            ok &= !cd.read(port + offset, 1, value) && value == 0xDEADBEEF;
            // No command, parameter, IRQ acknowledgement or mixer write is accepted.
            for (u32 input = 0; input < 256; ++input)
                ok &= !cd.write(port + offset, 1, input);
        }
        ok &= cd.bank() == bank && cd.status() == (0x18u | bank);
    }
    check("CD_banked_ports_remain_unsupported", ok && cd.writes == 260 && cd.reads == 257);

    Memory m = reset_test_memory();
    ok = !m.write(port, 1, 1) && !m.read(port, 1, value);
    m.attach_cdrom(&cd);
    ok &= m.write(0x9F801800, 1, 1) && m.read(0xBF801800, 1, value) && value == 0x19;
    ok &= m.write(0xBF801800, 1, 2) && m.read(port, 1, value) && value == 0x1A;
    ok &= !m.write(0xDF801800, 1, 0) && !m.read(0x3F801800, 1, value);
    m.write(port, 1, 1);
    Context c; c.start(0x8008B764);
    guest_read(c, m, port + 3, 1, value);
    ok &= c.stop == Stop::unmapped && c.bad_vaddr == port + 3 &&
          c.epc == 0x8008B764 && c.cause == 0x1C && cd.bank() == 1;
    Context command; command.start(0x80001004); command.in_delay = true;
    m.write(port, 1, 0);
    guest_write(command, m, port + 1, 1, 1);
    ok &= command.stop == Stop::unmapped && command.epc == 0x80001000 &&
          command.cause == 0x8000001C && cd.status() == 0x18;
    check("CD_MMIO_aliases_and_guest_faults", ok);
    return report;
}
} // namespace opengt::guest
