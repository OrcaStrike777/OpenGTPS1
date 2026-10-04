#pragma once
#include "opengt/guest.hpp"

namespace opengt::guest {
// Configuration writes only, as encountered in SDK IRQ initialization.
// Reads/counting/IRQ generation deliberately remain unresolved until timing
// is implemented. No host wall clock stands in for a PS1 timer.
struct TimerSetup {
    u32 mode[3]{}, counter[3]{}, writes{};
    bool write(u32 physical, unsigned width, u32 value) noexcept;
};

struct StartupBios {
    // Bounded TTY transcript, shared by host and ARM11 diagnostics. No output
    // is dropped: calls that cannot fit or read their input remain unresolved.
    static constexpr u32 console_capacity = 256;
    char console[console_capacity + 1]{};
    u32 console_size{}, console_column{}, puts_calls{}, printf_calls{};
    u32 hook_buffer{}, interrupt_environment{}, calls{};
    // Cold SIO0 policy (driver start enables it). Timer policies model the
    // initialized BIOS kernel at executable handoff, not power-on hardware.
    u32 pad_auto_ack{}, pad_calls{};
    u32 timer_auto_ack[4]{1, 1, 1, 1}, rcnt_calls{};
    // Bounded loader fixture: the five BIOS CD events initially exist. These
    // are lifecycle flags, not event handles or a general BIOS event table.
    u32 cd_events_open{0x1F}, cd_remove_calls{}, cd_close_attempts{};
    u32 cd_dequeue_attempts{};
    bool cd_dequeue_unresolved{}; // Retail SysDeqIntRP bug: no success invented.
    u32 syscall_calls{}, critical_entries{}, critical_exits{};
    u32 irq_entries{}, irq_returns{}, irq_deferred{}, irq_resume_pc{}, irq_hook_pc{};
    bool irq_active{};
    // Bounded HLE of the BIOS exit-hook handoff for IRQ0 with BIOS auto-ack
    // disabled. The guest handler/callback/acknowledgement execute normally.
    bool dispatch_interrupt(Context&, Memory&, InterruptController&) noexcept;
    // Called only after a verified pad/card VBlank handler, not on the BIOS
    // setter or each guest instruction. No pad polling/IRQ delivery is implied.
    bool acknowledge_pad_vblank(InterruptController&) const noexcept;
    // Returns false without changing guest state for every unsupported API.
    bool dispatch(Context&) noexcept;
    // A(3E)/B(3F) puts; A(3F) printf with %% and %[0][width]x/X only.
    // Preserves guest state and transcript on rejection.
    bool dispatch_console(Context&, const Memory&) noexcept;
    // Handles only ordinary SYS(1/2) traps. Unknown APIs and delay-slot
    // syscalls remain stopped with their original EPC/BD diagnostics.
    bool dispatch_syscall(Context&) noexcept;
private:
    Context interrupted_{}; // One fixed saved TCB projection; no nested IRQs.
};
} // namespace opengt::guest
