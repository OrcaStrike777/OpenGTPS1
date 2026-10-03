#pragma once
#include "opengt/guest.hpp"
#include "opengt/guest_tests.hpp"
namespace opengt::guest {
struct BootReport {
    bool available{}, passed{}, clears_verified{}, crossed_istat{};
    u32 entry{}, pc{}, last_pc{}, last_function{}, functions{}, instructions{};
    u32 unresolved{}, ra{}, sp{}, io_accesses{};
    u32 stat_reads{}, stat_writes{}, mask_reads{}, mask_writes{};
    u32 dma_reads{}, dma_writes{}, timer_writes{};
    u32 bios_api{}, bios_calls{}, hook_buffer{}, irq_pending{}, irq_mask{};
    u32 pad_auto_ack{}, vblank_auto_ack{}, pad_calls{}, rcnt_calls{};
    Stop stop{Stop::running};
    u32 trace[32]{}; unsigned trace_count{};
};
// devices=false retains the original unmapped-I_MASK diagnostic checkpoint.
// Stops synchronously within a bounded instruction budget; no host recursion.
BootReport run_boot_probe(bool devices = true, u32 budget = 2000000) noexcept;
TestReport run_boot_runtime_tests() noexcept;
TestReport run_interrupt_tests() noexcept;
TestReport run_bios_tests() noexcept;
}
