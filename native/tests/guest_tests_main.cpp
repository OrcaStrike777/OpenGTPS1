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
    const auto dma = opengt::guest::run_dma_tests();
    for (unsigned i=0;i<dma.count;++i)
        std::printf("%s %s\n",dma.tests[i].passed?"PASS":"FAIL",dma.tests[i].name);
    std::printf("DMA %u/%u\n",dma.passed,dma.count);
    if(dma.count!=dma.passed) return 1;
    const auto gpu = opengt::guest::run_gpu_tests();
    for (unsigned i=0;i<gpu.count;++i)
        std::printf("%s %s\n",gpu.tests[i].passed?"PASS":"FAIL",gpu.tests[i].name);
    std::printf("GPU %u/%u\n",gpu.passed,gpu.count);
    if(gpu.count!=gpu.passed) return 1;
    const auto cd = opengt::guest::run_cdrom_tests();
    for (unsigned i=0;i<cd.count;++i)
        std::printf("%s %s\n",cd.tests[i].passed?"PASS":"FAIL",cd.tests[i].name);
    std::printf("CD %u/%u\n",cd.passed,cd.count);
    if(cd.count!=cd.passed) return 1;
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
        std::printf("DICR R=%lu W=%lu state=%08lx rises=%lu completions=%lu channels R=%lu W=%lu IRQ=%03lx/%03lx\n",
            (unsigned long)boot.dicr_reads, (unsigned long)boot.dicr_writes, (unsigned long)boot.dicr_state,
            (unsigned long)boot.dma_irq_rises, (unsigned long)boot.dma_completions,
            (unsigned long)boot.dma_channel_reads, (unsigned long)boot.dma_channel_writes,
            (unsigned long)boot.irq_pending, (unsigned long)boot.irq_mask);
        for (unsigned i=0;i<boot.trace_count;++i) std::printf(" %08lx",(unsigned long)boot.trace[i]);
        std::puts("");
        std::printf("CD remove=%lu events=%02lx close=%lu dequeue=%lu unresolved=%d; SYS calls=%lu enter=%lu exit=%lu SR=%08lx Cause=%08lx EPC=%08lx\n",
            (unsigned long)boot.cd_remove_calls, (unsigned long)boot.cd_events_open,
            (unsigned long)boot.cd_close_attempts, (unsigned long)boot.cd_dequeue_attempts, boot.cd_dequeue_unresolved,
            (unsigned long)boot.syscall_calls, (unsigned long)boot.critical_entries, (unsigned long)boot.critical_exits,
            (unsigned long)boot.sr, (unsigned long)boot.cause, (unsigned long)boot.epc);
        std::printf("TTY puts=%lu printf=%lu bytes=%lu hex=", (unsigned long)boot.puts_calls, (unsigned long)boot.printf_calls, (unsigned long)boot.console_size);
        for (unsigned i=0;i<boot.console_size;++i)
            std::printf("%02x", static_cast<unsigned>(static_cast<unsigned char>(boot.console[i])));
        std::puts("");
        std::printf("VBlank wait=%d callback=%08lx counter=%lu polls=%lu\n",boot.waiting_vblank,
            (unsigned long)boot.vblank_callback,(unsigned long)boot.vblank_counter,(unsigned long)boot.vblank_polls);
        std::printf("VBlank edges=%lu phase=%lu IRQ entries=%lu returns=%lu deferred=%lu active=%d resume=%08lx hookPC=%08lx callback=%d SDK=%lu busy=%lu GPU=%08lx\n",
            (unsigned long)boot.vblank_edges,(unsigned long)boot.vblank_phase,(unsigned long)boot.irq_entries,
            (unsigned long)boot.irq_returns,(unsigned long)boot.irq_deferred,boot.irq_active,
            (unsigned long)boot.irq_resume_pc,(unsigned long)boot.irq_hook_pc,boot.entered_vblank_callback,
            (unsigned long)boot.sdk_vblank_counter,(unsigned long)boot.guest_in_interrupt,(unsigned long)boot.gpu_control_value);
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
        const auto dicr_stop = opengt::guest::run_boot_probe(true, 1307516);
        const auto dicr_crossed = opengt::guest::run_boot_probe(true, 1307517);
        if (dicr_stop.stop != opengt::guest::Stop::budget || dicr_stop.pc != 0x8008C698 ||
            dicr_stop.instructions != 1307513 || dicr_stop.functions != 20 ||
            dicr_stop.crossed_dicr || dicr_stop.dicr_writes != 0 ||
            dicr_crossed.stop != opengt::guest::Stop::budget || dicr_crossed.pc != 0x8008BCA8 ||
            dicr_crossed.instructions != 1307514 || !dicr_crossed.crossed_dicr ||
            dicr_crossed.dicr_writes != 1 || dicr_crossed.dicr_state != 0) return 1;
        std::puts("PASS DICR delay-slot store watchdog checkpoints");
        const auto cd_stop = opengt::guest::run_boot_probe(true, 1307599);
        const auto cd_return = opengt::guest::run_boot_probe(true, 1307600);
        const auto sys_before = opengt::guest::run_boot_probe(true, 1307603);
        const auto sys_trap = opengt::guest::run_boot_probe(true, 1307604);
        const auto sys_return = opengt::guest::run_boot_probe(true, 1307605);
        const auto reset_return = opengt::guest::run_boot_probe(true, 1307616);
        if (cd_stop.pc != 0xA0 || cd_stop.instructions != 1307596 || cd_stop.cd_remove_calls != 0 ||
            cd_return.pc != 0x8008BEC8 || cd_return.instructions != 1307596 ||
            cd_return.cd_remove_calls != 1 || cd_return.cd_events_open != 0 || cd_return.critical_entries != 1 ||
            (cd_return.sr & 0x401) != 0 || sys_before.pc != 0x8008C94C ||
            sys_before.instructions != 1307599 || sys_before.syscall_calls != 0 ||
            sys_trap.pc != 0x8008C94C || sys_trap.instructions != 1307600 ||
            sys_trap.cause != 0x20 || sys_trap.epc != 0x8008C94C || sys_trap.syscall_calls != 0 ||
            sys_return.pc != 0x8008C950 || sys_return.instructions != 1307600 ||
            sys_return.syscall_calls != 1 || sys_return.sr != 0x401 || sys_return.last_pc != 0x8008C94C ||
            reset_return.pc != 0x800109B0 || reset_return.instructions != 1307611 || reset_return.functions != 24)
            return 1;
        std::puts("PASS CD cleanup / SYS trap-resume / ResetCallback checkpoints");
        const auto wait_start = opengt::guest::run_boot_probe(true, 1307648);
        const auto wait_loop = opengt::guest::run_boot_probe(true, 1307653);
        const auto unscheduled = opengt::guest::run_boot_probe(true, 2000000, false);
        if (!wait_start.waiting_vblank || wait_start.pc != 0x8001096C || wait_start.instructions != 1307643 ||
            wait_start.vblank_counter != 0 || wait_start.vblank_polls != 0 ||
            !wait_loop.waiting_vblank || wait_loop.pc != wait_start.pc ||
            wait_loop.vblank_counter != 0 || wait_loop.vblank_polls != 1 || wait_loop.instructions != 1307648 ||
            !unscheduled.passed || unscheduled.vblank_edges != 0 || unscheduled.irq_entries != 0 ||
            unscheduled.vblank_callback != 0x80010928 || unscheduled.vblank_counter != 0 || unscheduled.vblank_polls != 138471)
            return 1;
        std::puts("PASS original VBlank callback registration and persistent wait");
        const auto irq_before = opengt::guest::run_boot_probe(true, 1698328);
        const auto irq_entered = opengt::guest::run_boot_probe(true, 1698329);
        const auto first_return = opengt::guest::run_boot_probe(true, 2000000);
        const auto gpu_after = opengt::guest::run_boot_probe(true, 1698439);
        const auto gpu_before = opengt::guest::run_boot_probe(true, 1698438);
        if (irq_before.stop != opengt::guest::Stop::budget || irq_before.pc != 0x8001096C ||
            irq_before.instructions != 1698323 || irq_before.vblank_edges != 3 || irq_before.irq_pending != 1 ||
            irq_before.irq_entries != 0 || irq_before.sr != 0x401 ||
            irq_entered.stop != opengt::guest::Stop::budget || irq_entered.pc != 0x8008BE74 ||
            irq_entered.instructions != irq_before.instructions || irq_entered.irq_entries != 1 ||
            !irq_entered.irq_active || irq_entered.sr != 0x404 || irq_entered.epc != irq_before.pc ||
            irq_entered.irq_pending != 1 || irq_entered.stat_writes != irq_before.stat_writes ||
            gpu_before.stop != opengt::guest::Stop::budget || gpu_before.pc != 0x8007F844 ||
            gpu_before.instructions != 1698432 || gpu_before.irq_pending != 0 || gpu_before.stat_writes != 2 ||
            !gpu_before.entered_vblank_callback || gpu_before.gpu_writes != 0 ||
            gpu_after.stop != opengt::guest::Stop::budget || gpu_after.pc != 0x80010938 ||
            gpu_after.instructions != gpu_before.instructions + 1 || gpu_after.gpu_writes != 1 ||
            gpu_after.gpu_status != 0x14802000 || gpu_after.cause != 0 || !gpu_after.irq_active)
            return 1;
        if (first_return.stop != opengt::guest::Stop::budget || !first_return.waiting_vblank ||
            first_return.irq_returns != 1 || first_return.irq_active || first_return.guest_in_interrupt != 0 ||
            first_return.vblank_counter != 1 || first_return.sdk_vblank_counter != 1 ||
            first_return.gpu_writes != 1 || first_return.sr != 0x401) return 1;
        if (boot.irq_returns != 4 || boot.irq_active || boot.vblank_counter != 4 ||
            boot.vblank_callback != 0 || boot.gpu_writes != 4 || boot.gpu_reads != 0 ||
            boot.gpu_status != 0x14802000 || boot.pc != 0x8008B764 || boot.unresolved != 0x1F801803 ||
            boot.puts_calls != 1 || boot.printf_calls != 1 || boot.console_size != 23) return 1;
        const auto puts_before = opengt::guest::run_boot_probe(true, 3396902);
        const auto puts_after = opengt::guest::run_boot_probe(true, 3396903);
        if (puts_before.stop != opengt::guest::Stop::budget || puts_before.pc != 0xB0 ||
            puts_before.instructions != 3396889 || puts_before.puts_calls || puts_before.console_size ||
            puts_after.stop != opengt::guest::Stop::budget || puts_after.pc != 0x8008B6F0 ||
            puts_after.instructions != puts_before.instructions || puts_after.puts_calls != 1 ||
            puts_after.console_size != 8 || puts_after.bios_calls != puts_before.bios_calls + 1 ||
            puts_after.vblank_phase != puts_before.vblank_phase) return 1;
        const auto printf_before = opengt::guest::run_boot_probe(true, 3396911);
        const auto printf_after = opengt::guest::run_boot_probe(true, 3396912);
        const auto cd_before = opengt::guest::run_boot_probe(true, 3397026);
        if (printf_before.stop != opengt::guest::Stop::budget || printf_before.pc != 0xA0 ||
            printf_before.instructions != 3396897 || printf_before.printf_calls || printf_before.console_size != 8 ||
            printf_after.stop != opengt::guest::Stop::budget || printf_after.pc != 0x8008B704 ||
            printf_after.instructions != printf_before.instructions || printf_after.printf_calls != 1 ||
            printf_after.console_size != 23 || printf_after.bios_calls != printf_before.bios_calls + 1 ||
            printf_after.vblank_phase != printf_before.vblank_phase) return 1;
        const auto cd_after = opengt::guest::run_boot_probe(true, 3397027);
        const auto cd_read_before = opengt::guest::run_boot_probe(true, 3397030);
        if (cd_after.stop != opengt::guest::Stop::budget || cd_after.pc != 0x8008B758 ||
            cd_after.instructions != 3397012 || cd_after.cd_writes != 1 || cd_after.cd_bank != 1 ||
            cd_after.cd_status != 0x19 || cd_after.cd_reads != 0 || cd_after.cause != 0 ||
            cd_read_before.stop != opengt::guest::Stop::budget || cd_read_before.pc != boot.pc ||
            cd_read_before.instructions + 1 != boot.instructions || cd_read_before.cause != 0 ||
            boot.cd_writes != 1 || boot.cd_reads != 0 || boot.cd_bank != 1 || boot.cd_status != 0x19) return 1;
        if (cd_before.stop != opengt::guest::Stop::budget || cd_before.pc != 0x8008B754 ||
            cd_before.instructions + 1 != cd_after.instructions || cd_before.cd_writes != 0 || cd_before.cd_bank != 0 || cd_before.irq_mask != 13 ||
            cd_before.mask_writes != 7 || cd_before.cause != 0 ||
            boot.stop != opengt::guest::Stop::unmapped || boot.cause != 0x1C || boot.epc != boot.pc) return 1;
        std::puts("PASS printf capture / guest return / CD index store / IRQ flags read checkpoints");
        std::puts("PASS puts entry / captured output / guest return checkpoints");
        std::puts("PASS four guest callbacks / BIOS returns / CdInit CD-ROM boundary");
        std::puts("PASS IRQ hook / guest ack / callback / GPU delay-slot continuation checkpoints");
        if (!boot.passed || !unshimmed.passed || bounded.stop != opengt::guest::Stop::budget || bounded.instructions != 32) return 1;
    }
    return 0;
}
