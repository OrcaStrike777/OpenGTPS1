#pragma once
#include "opengt/guest.hpp"

namespace opengt::guest {
// Bounded idle GPU fixture: reset drawing/display mode, no GP0 work accepted.
// Only GP1(03h) is implemented. Other commands/transfers must still trap.
// The fixed NTSC clock's phase zero is the start of VBlank (line 256).
class GpuControl {
public:
    explicit GpuControl(const VBlankClock& clock) noexcept : clock_(clock) {}
    bool read(u32 physical, unsigned width, u32& value) noexcept;
    bool write(u32 physical, unsigned width, u32 value) noexcept;
    u32 status() const noexcept;
    u32 reads{}, writes{}, last_command{};
private:
    const VBlankClock& clock_;
    bool display_disabled_{true};
};
} // namespace opengt::guest
