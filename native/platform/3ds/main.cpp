#include "host.hpp"
#include "opengt/stdio_files.hpp"
#include "opengt/guest_tests.hpp"

#include <cstdio>

int main() {
    opengt::ctr::Host host;
    if (!host.initialize()) {
        std::printf("Press START to exit.\n");
        while (host.running()) {
            if (host.poll().pressed & opengt::platform::start) break;
            gfxFlushBuffers();
            host.wait_vblank();
        }
        return 1;
    }

    // libctru's default app startup mounts sdmc for newlib file I/O.
    opengt::platform::StdioFiles files("sdmc:/3ds/opengtps1");
    opengt::platform::SilentAudio audio;
    opengt::platform::UnavailableSaves saves;
    (void)saves; // Reserved boundary; bootstrap never writes a memory card.
    opengt::platform::Diagnostics diagnostics{};
    auto guest_report = opengt::guest::run_synthetic_tests();
    opengt::guest::TestReport gt2_report{};
    if (guest_report.passed == guest_report.count) gt2_report = opengt::guest::run_gt2_probe();
    diagnostics.guest_tests = &guest_report;
    diagnostics.gt2_probe = &gt2_report;
    char probe[64]{};
    std::size_t bytes_read = 0;
    diagnostics.storage = files.read_at("bootstrap.txt", 0, probe, sizeof(probe), bytes_read);
    std::uint64_t previous = host.ticks();
    while (host.running()) {
        // Exactly one host VBlank wait, separate from future PS1 scheduling.
        host.wait_vblank();
        diagnostics.input = host.poll();
        if (diagnostics.input.pressed & opengt::platform::start) break;
        if (diagnostics.input.pressed & opengt::platform::x) {
            guest_report = opengt::guest::run_synthetic_tests();
            gt2_report = {};
            if (guest_report.passed == guest_report.count) gt2_report = opengt::guest::run_gt2_probe();
        }
        const auto now = host.ticks();
        diagnostics.frame_ms = 1000.0 * static_cast<double>(now - previous) /
                               static_cast<double>(host.ticks_per_second());
        previous = now;
        diagnostics.linear_free_bytes = host.linear_free_bytes();
        if (!host.present(diagnostics)) {
            audio.stop();
            return 2;
        }
        ++diagnostics.frame;
    }
    audio.stop();
    return 0;
}
