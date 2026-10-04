#pragma once
#include "opengt/guest.hpp"
#include "opengt/guest_tests.hpp"
namespace opengt::guest {
struct BootReport {
    bool available{}, passed{}, clears_verified{}, crossed_istat{}, crossed_dicr{};
    u32 entry{}, pc{}, last_pc{}, last_function{}, functions{}, instructions{};
    u32 unresolved{}, ra{}, sp{}, io_accesses{};
    u32 stat_reads{}, stat_writes{}, mask_reads{}, mask_writes{};
    u32 dma_reads{}, dma_writes{}, timer_writes{};
    u32 bios_api{}, bios_calls{}, hook_buffer{}, irq_pending{}, irq_mask{};
    u32 pad_auto_ack{}, vblank_auto_ack{}, pad_calls{}, rcnt_calls{};
    u32 dicr_reads{}, dicr_writes{}, dicr_state{}, dma_completions{}, dma_irq_rises{};
    u32 dma_channel_reads{}, dma_channel_writes{};
    u32 sr{}, cause{}, epc{}, syscall_api{}, syscall_calls{}, critical_entries{}, critical_exits{};
    u32 cd_remove_calls{}, cd_events_open{}, cd_close_attempts{}, cd_dequeue_attempts{};
    bool cd_dequeue_unresolved{};
    u32 vblank_callback{}, vblank_counter{}, vblank_polls{};
    bool waiting_vblank{};
    u32 vblank_edges{}, vblank_phase{}, irq_entries{}, irq_returns{}, irq_deferred{};
    u32 irq_resume_pc{}, irq_hook_pc{}, gpu_control_value{};
    bool irq_active{}, entered_vblank_callback{};
    u32 sdk_vblank_counter{}, guest_in_interrupt{};
    u32 gpu_writes{}, gpu_reads{}, gpu_status{};
    u32 puts_calls{}, printf_calls{}, console_size{};
    char console[257]{};
    Stop stop{Stop::running};
    u32 trace[64]{}; unsigned trace_count{};
};
// devices=false retains the original unmapped-I_MASK diagnostic checkpoint.
// schedule_vblank=false preserves the 88e4fc2 counter-wait regression fixture.
// Stops synchronously within a bounded instruction budget; no host recursion.
BootReport run_boot_probe(bool devices = true, u32 budget = 4000000, bool schedule_vblank = true) noexcept;
TestReport run_boot_runtime_tests() noexcept;
TestReport run_interrupt_tests() noexcept;
TestReport run_bios_tests() noexcept;
TestReport run_dma_tests() noexcept;
TestReport run_gpu_tests() noexcept;
}
