#include "opengt/guest_tests.hpp"
#include <cstdio>
int main() {
    const auto report = opengt::guest::run_synthetic_tests();
    for (unsigned i = 0; i < report.count; ++i) {
        const auto& t = report.tests[i];
        std::printf("%s %-24s actual=%08lx expected=%08lx\n", t.passed ? "PASS" : "FAIL",
                    t.name, static_cast<unsigned long>(t.actual), static_cast<unsigned long>(t.expected));
    }
    std::printf("MIPS %u/%u signature=%08lx\n", report.passed, report.count,
                static_cast<unsigned long>(report.signature));
    if (report.passed != report.count) return 1;
    const auto gt2 = opengt::guest::run_gt2_probe();
    for (unsigned i = 0; i < gt2.count; ++i) {
        const auto& t = gt2.tests[i];
        std::printf("%s GT2 %-20s actual=%08lx expected=%08lx\n", t.passed ? "PASS" : "FAIL",
                    t.name, static_cast<unsigned long>(t.actual), static_cast<unsigned long>(t.expected));
    }
    if (gt2.count) std::printf("GT2 %u/%u signature=%08lx\n", gt2.passed, gt2.count,
                              static_cast<unsigned long>(gt2.signature));
    return gt2.count == gt2.passed ? 0 : 1;
}
