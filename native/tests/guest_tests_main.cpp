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
    const auto interrupts = opengt::guest::run_interrupt_tests();
    for (unsigned i=0;i<interrupts.count;++i)
        std::printf("%s %s\n",interrupts.tests[i].passed?"PASS":"FAIL",interrupts.tests[i].name);
    std::printf("IRQ %u/%u\n",interrupts.passed,interrupts.count);
    if(interrupts.count!=interrupts.passed) return 1;
    const auto bios = opengt::guest::run_bios_tests();
    for (unsigned i=0;i<bios.count;++i)
        std::printf("%s %s\n",bios.tests[i].passed?"PASS":"FAIL",bios.tests[i].name);
    std::printf("BIOS %u/%u\n",bios.passed,bios.count);
    if(bios.count!=bios.passed) return 1;
    const auto boot = opengt::guest::run_boot_probe();
    if (boot.available) {
        std::printf("BOOT %s entry=%08lx pc=%08lx last=%08lx func=%08lx calls=%lu instructions=%lu boundary=%08lx reason=%s clear=%d IO=%lu\n",
            boot.passed ? "PASS" : "FAIL", (unsigned long)boot.entry, (unsigned long)boot.pc,
            (unsigned long)boot.last_pc, (unsigned long)boot.last_function, (unsigned long)boot.functions,
            (unsigned long)boot.instructions, (unsigned long)boot.unresolved, opengt::guest::stop_name(boot.stop),
            boot.clears_verified, (unsigned long)boot.io_accesses);
        std::printf("IRQ stat R=%lu W=%lu mask R=%lu W=%lu; DMA R=%lu W=%lu; timer W=%lu; BIOS calls=%lu API=%02lx hook=%08lx crossed=%d\n",
            (unsigned long)boot.stat_reads, (unsigned long)boot.stat_writes, (unsigned long)boot.mask_reads,
            (unsigned long)boot.mask_writes, (unsigned long)boot.dma_reads, (unsigned long)boot.dma_writes,
            (unsigned long)boot.timer_writes, (unsigned long)boot.bios_calls, (unsigned long)boot.bios_api,
            (unsigned long)boot.hook_buffer, boot.crossed_istat);
        std::printf("PAD calls=%lu flag=%lu; RCnt calls=%lu VBlank flag=%lu\n",
            (unsigned long)boot.pad_calls, (unsigned long)boot.pad_auto_ack,
            (unsigned long)boot.rcnt_calls, (unsigned long)boot.vblank_auto_ack);
        for (unsigned i=0;i<boot.trace_count;++i) std::printf(" %08lx",(unsigned long)boot.trace[i]);
        std::puts("");
        const auto unshimmed = opengt::guest::run_boot_probe(false);
        const auto bounded = opengt::guest::run_boot_probe(true, 32);
        std::printf("Unshimmed %s PC=%08lx boundary=%08lx; watchdog %s instructions=%lu\n",
            unshimmed.passed ? "PASS" : "FAIL", (unsigned long)unshimmed.pc, (unsigned long)unshimmed.unresolved,
            opengt::guest::stop_name(bounded.stop), (unsigned long)bounded.instructions);
        const auto hook_stop = opengt::guest::run_boot_probe(true, 1307316);
        const auto hook_return = opengt::guest::run_boot_probe(true, 1307317);
        std::printf("BIOS watchdog: before=%lu calls/%lu instructions after=%lu calls/%lu instructions\n",
            (unsigned long)hook_stop.bios_calls, (unsigned long)hook_stop.instructions,
            (unsigned long)hook_return.bios_calls, (unsigned long)hook_return.instructions);
        if (hook_stop.stop != opengt::guest::Stop::budget || hook_stop.pc != 0xB0 ||
            hook_stop.bios_calls != 0 || hook_stop.instructions != 1307316 ||
            hook_return.stop != opengt::guest::Stop::budget || hook_return.pc != 0x8008BE9C ||
            hook_return.bios_calls != 1 || hook_return.instructions != 1307316) return 1;
        const auto pad_stop = opengt::guest::run_boot_probe(true, 1307425);
        const auto pad_return = opengt::guest::run_boot_probe(true, 1307426);
        const auto rcnt_stop = opengt::guest::run_boot_probe(true, 1307432);
        const auto rcnt_return = opengt::guest::run_boot_probe(true, 1307433);
        if (pad_stop.stop != opengt::guest::Stop::budget || pad_stop.pc != 0xB0 ||
            pad_stop.instructions != 1307424 || pad_stop.functions != 17 || pad_stop.pad_calls != 0 ||
            pad_return.stop != opengt::guest::Stop::budget || pad_return.pc != 0x8008C178 ||
            pad_return.instructions != 1307424 || pad_return.bios_calls != 2 ||
            pad_return.pad_calls != 1 || pad_return.pad_auto_ack != 0 ||
            rcnt_stop.stop != opengt::guest::Stop::budget || rcnt_stop.pc != 0xC0 ||
            rcnt_stop.instructions != 1307430 || rcnt_stop.rcnt_calls != 0 || rcnt_stop.vblank_auto_ack != 1 ||
            rcnt_return.stop != opengt::guest::Stop::budget || rcnt_return.pc != 0x8008C184 ||
            rcnt_return.instructions != 1307430 || rcnt_return.bios_calls != 3 ||
            rcnt_return.rcnt_calls != 1 || rcnt_return.vblank_auto_ack != 0) return 1;
        std::puts("PASS ChangeClearPAD/RCnt entry-return watchdog checkpoints");
        if (!boot.passed || !unshimmed.passed || bounded.stop != opengt::guest::Stop::budget || bounded.instructions != 32) return 1;
    }
    return 0;
}
