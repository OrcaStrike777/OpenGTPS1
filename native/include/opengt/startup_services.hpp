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
    // The one reached API with a usable desktop implementation: B(19h).
    // Returns false without changing guest state for every unsupported API.
    bool dispatch(Context&) noexcept;
};
} // namespace opengt::guest
