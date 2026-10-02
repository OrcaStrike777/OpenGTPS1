#include "opengt/guest_tests.hpp"
#include "opengt/boot_probe.hpp"
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
    if (gt2.count != gt2.passed) return 1;
    const auto operations = opengt::guest::run_boot_runtime_tests();
    for (unsigned i=0;i<operations.count;++i)
        std::printf("%s %s\n",operations.tests[i].passed?"PASS":"FAIL",operations.tests[i].name);
    if(operations.count!=operations.passed) return 1;
    const auto boot = opengt::guest::run_boot_probe();
    if (boot.available) {
        std::printf("BOOT %s entry=%08lx pc=%08lx last=%08lx func=%08lx calls=%lu instructions=%lu boundary=%08lx reason=%s clear=%d IO=%lu\n",
            boot.passed ? "PASS" : "FAIL", (unsigned long)boot.entry, (unsigned long)boot.pc,
            (unsigned long)boot.last_pc, (unsigned long)boot.last_function, (unsigned long)boot.functions,
            (unsigned long)boot.instructions, (unsigned long)boot.unresolved, opengt::guest::stop_name(boot.stop),
            boot.clears_verified, (unsigned long)boot.io_accesses);
        for (unsigned i=0;i<boot.trace_count;++i) std::printf(" %08lx",(unsigned long)boot.trace[i]);
        std::puts("");
        const auto unshimmed = opengt::guest::run_boot_probe(false);
        const auto bounded = opengt::guest::run_boot_probe(true, 32);
        std::printf("Unshimmed %s PC=%08lx boundary=%08lx; watchdog %s instructions=%lu\n",
            unshimmed.passed ? "PASS" : "FAIL", (unsigned long)unshimmed.pc, (unsigned long)unshimmed.unresolved,
            opengt::guest::stop_name(bounded.stop), (unsigned long)bounded.instructions);
        if (!boot.passed || !unshimmed.passed || bounded.stop != opengt::guest::Stop::budget || bounded.instructions != 32) return 1;
    }
    return 0;
}
