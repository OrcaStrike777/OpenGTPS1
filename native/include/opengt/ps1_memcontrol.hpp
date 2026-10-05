#pragma once
#include "opengt/guest.hpp"

namespace opengt::guest {
// COM_DELAY latch only. Other memory-control ports and bus wait states are
// not modeled. Initial value is the common BIOS-style timing configuration.
class CommonDelay {
public:
    bool read(u32 physical, unsigned width, u32& value) noexcept;
    bool write(u32 physical, unsigned width, u32 value) noexcept;
    u32 value() const noexcept { return value_; }
    u32 reads{}, writes{};
private:
    u32 value_{0x00031125};
};
} // namespace opengt::guest
