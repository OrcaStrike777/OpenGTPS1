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
    u32 hook_buffer{}, interrupt_environment{}, calls{};
    // Cold SIO0 policy (driver start enables it). Timer policies model the
    // initialized BIOS kernel at executable handoff, not power-on hardware.
    u32 pad_auto_ack{}, pad_calls{};
    u32 timer_auto_ack[4]{1, 1, 1, 1}, rcnt_calls{};
    // Called only after a verified pad/card VBlank handler, not on the BIOS
    // setter or each guest instruction. No pad polling/IRQ delivery is implied.
    bool acknowledge_pad_vblank(InterruptController&) const noexcept;
    // Returns false without changing guest state for every unsupported API.
    bool dispatch(Context&) noexcept;
};
} // namespace opengt::guest
