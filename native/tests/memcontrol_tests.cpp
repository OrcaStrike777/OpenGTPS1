#include "opengt/boot_probe.hpp"
#include "opengt/ps1_memcontrol.hpp"
#include "opengt/ps1_cdrom.hpp"
#include <initializer_list>

namespace opengt::guest {
TestReport run_memcontrol_tests() noexcept {
    TestReport report{};
    const auto check = [&](const char* name, bool ok) {
        report.tests[report.count++] = {name, ok, ok ? 1u : 0u, 1};
        report.passed += ok;
    };
    constexpr u32 port = 0x1F801020;
    CommonDelay timing;
    u32 value = 0;
    bool ok = timing.read(port, 4, value) && value == 0x31125;
    for (unsigned bit = 0; bit < 32; ++bit) {
        ok &= timing.write(port, 4, u32{1} << bit) && timing.read(port, 4, value);
        ok &= value == (bit < 18 ? u32{1} << bit : 0);
    }
    ok &= timing.write(port, 4, 0xFFFFFFFF) && timing.read(port, 4, value) && value == 0x3FFFF;
    ok &= timing.write(port, 4, 0x1325) && timing.read(port, 4, value) && value == 0x1325;
    ok &= timing.write(port, 4, 0) && timing.read(port, 4, value) && value == 0;
    check("MC_word_replace_readback_mask", ok && timing.writes == 35 && timing.reads == 36);

    timing.write(port, 4, 0x21234);
    ok = true;
    for (unsigned width : {0u, 1u, 2u, 3u, 8u}) {
        value = 0xDEADBEEF;
        ok &= !timing.write(port, width, 0) && !timing.read(port, width, value);
        ok &= value == 0xDEADBEEF;
    }
    for (u32 address : {port - 4, port + 1, port + 2, port + 3, port + 4}) {
        value = 0xDEADBEEF;
        ok &= !timing.write(address, 4, 0) && !timing.read(address, 4, value);
        ok &= value == 0xDEADBEEF;
    }
    check("MC_unsupported_access_atomic", ok && timing.value() == 0x21234 &&
          timing.writes == 36 && timing.reads == 36);

    Memory m = reset_test_memory();
    ok = !m.read(port, 4, value) && !m.write(port, 4, 0x1325);
    m.attach_common_delay(&timing);
    ok &= m.write(0xBF801020, 4, 0x31234) && m.read(0x9F801020, 4, value) && value == 0x31234;
    ok &= m.write(0x9F801020, 4, 0x1325) && m.read(port, 4, value) && value == 0x1325;
    ok &= !m.write(0xDF801020, 4, 0) && !m.read(0x3F801020, 4, value);
    check("MC_aliases_attachment", ok && timing.value() == 0x1325);

    Context c; c.start(0x8008B81C);
    c.write(31, 0x8008B824); c.begin(); c.branch(0x8008B034);
    c.pc = 0x8008B820;
    const u32 target = c.begin();
    guest_write(c, m, port, 4, 0x1325);
    ok = c.stop == Stop::running && c.in_delay && target == 0x8008B034 && c.cause == 0;
    ok &= timing.value() == 0x1325 && c.read(31) == 0x8008B824;
    Context unsupported; unsupported.start(0x8008B820); unsupported.in_delay = true;
    guest_write(unsupported, m, port - 4, 4, 0x1325);
    ok &= unsupported.stop == Stop::unmapped && unsupported.cause == 0x8000001C &&
          unsupported.epc == 0x8008B81C && unsupported.bad_vaddr == port - 4;
    Context misaligned; misaligned.start(0x80001000);
    guest_write(misaligned, m, port + 1, 4, 0);
    ok &= misaligned.stop == Stop::address_store && timing.value() == 0x1325;
    check("MC_delay_slot_and_faults", ok);

    CdromRegisters cd; InterruptController irq;
    m.attach_cdrom(&cd); m.attach_interrupts(&irq);
    irq.pulse(2); cd.interrupt_flags = 3;
    ok = m.write(port, 4, 0x1325) && cd.status() == 0x18 && cd.interrupt_flags == 3;
    ok &= irq.pending() == 4 && cd.reads == 0 && cd.writes == 0;
    ok &= !m.write(0x1F801801, 1, 1) && !m.read(0x1F801802, 1, value);
    ok &= !m.read(0x1F801110, 4, value); // Next real device is not silently implemented.
    check("MC_no_device_completion", ok);
    return report;
}
} // namespace opengt::guest
