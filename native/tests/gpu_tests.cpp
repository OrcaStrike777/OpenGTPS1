#include "opengt/boot_probe.hpp"
#include "opengt/ps1_gpu.hpp"
#include <initializer_list>

namespace opengt::guest {
TestReport run_gpu_tests() noexcept {
    TestReport report{};
    const auto check = [&](const char* name, bool ok) {
        report.tests[report.count++] = {name, ok, ok ? 1u : 0u, 1};
        report.passed += ok;
    };
    constexpr u32 port = 0x1F801814;
    VBlankClock clock{};
    GpuControl gpu(clock);
    u32 value = 0;
    check("GPU_idle_status", gpu.read(port, 4, value) && value == 0x14802000 &&
          gpu.reads == 1 && gpu.writes == 0);
    bool ok = gpu.write(port, 4, 0x03000000) && gpu.status() == 0x14002000;
    ok &= gpu.write(port, 4, 0x03FFFFFE) && gpu.status() == 0x14002000;
    ok &= gpu.write(port, 4, 0x03000001) && gpu.status() == 0x14802000;
    ok &= gpu.write(port, 4, 0xC3000000) && gpu.status() == 0x14002000;
    ok &= gpu.writes == 4 && gpu.last_command == 0xC3000000;
    check("GPU_display_mask_and_mirror", ok);

    ok = true;
    for (u32 command = 0; command < 256; ++command) {
        if ((command & 0x3F) == 3) continue;
        ok &= !gpu.write(port, 4, command << 24);
    }
    for (unsigned width : {0u, 1u, 2u, 3u, 8u}) {
        value = 0xDEADBEEF;
        ok &= !gpu.write(port, width, 0x03000001) && !gpu.read(port, width, value);
        ok &= value == 0xDEADBEEF;
    }
    ok &= !gpu.write(port + 1, 4, 0x03000001) && !gpu.read(port + 1, 4, value);
    ok &= !gpu.write(port - 4, 4, 0) && !gpu.read(port - 4, 4, value);
    ok &= gpu.writes == 4 && gpu.reads == 1 && gpu.status() == 0x14002000;
    check("GPU_unsupported_access_is_atomic", ok);

    Memory m = reset_test_memory();
    ok = !m.write(port, 4, 0x03000001);
    m.attach_gpu_control(&gpu);
    ok &= m.write(0x9F801814, 4, 0x03000001) && m.read(0xBF801814, 4, value);
    ok &= value == 0x14802000 && m.write(port, 4, 0x03000000);
    ok &= !m.write(0xDF801814, 4, 0x03000001) && !m.read(0x3F801814, 4, value);
    Context c; c.start(0x80001004); c.in_delay = true;
    guest_write(c, m, port, 4, 0x04000002); // DMA direction is still unsupported.
    ok &= c.stop == Stop::unmapped && c.epc == 0x80001000 && c.cause == 0x8000001C;
    ok &= gpu.status() == 0x14002000;
    check("GPU_MMIO_aliases_and_fault", ok);

    InterruptController irq;
    ok = true;
    // Known phase offsets from VBlank start: +23 lines is visible line 16,
    // +24 is odd line 17; +262 is the final visible line 255.
    // Display disable never suppresses the clock or its IRQ0 edges.
    gpu.write(port, 4, 0x03000001);
    const struct { u32 phase; bool odd; } samples[] = {
        {0, false}, {6 * 4305, false}, {7 * 4305, false},
        {23 * 4305 - 1, false}, {23 * 4305, false},
        {24 * 4305 - 1, false}, {24 * 4305, true},
        {25 * 4305 - 1, true}, {25 * 4305, false},
        {261 * 4305, false}, {262 * 4305, true}
    };
    for (const auto& sample : samples) {
        clock.phase = sample.phase;
        ok &= gpu.read(port, 4, value) && value == (sample.odd ? 0x94802000u : 0x14802000u);
        ok &= gpu.status() == value; // Reads must not advance video time.
    }
    clock.phase = VBlankClock::field_half_cycles - 1;
    clock.advance(1, irq);
    ok &= clock.edges == 1 && irq.pending() == 1 && gpu.status() == 0x14802000;
    check("GPU_scanline_status_and_VBlank", ok);
    return report;
}
} // namespace opengt::guest
