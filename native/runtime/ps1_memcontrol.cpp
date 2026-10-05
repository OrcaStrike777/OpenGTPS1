#include "opengt/ps1_memcontrol.hpp"

namespace opengt::guest {
bool CommonDelay::read(u32 physical, unsigned width, u32& value) noexcept {
    if (physical != 0x1F801020 || width != 4) return false;
    value = value_;
    ++reads;
    return true;
}
bool CommonDelay::write(u32 physical, unsigned width, u32 value) noexcept {
    if (physical != 0x1F801020 || width != 4) return false;
    // COM0..3 and the two unknown R/W bits survive; bits 18..31 read zero.
    value_ = value & 0x0003FFFF;
    ++writes;
    return true;
}
} // namespace opengt::guest
