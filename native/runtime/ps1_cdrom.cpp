#include "opengt/ps1_cdrom.hpp"

namespace opengt::guest {
bool CdromRegisters::read(u32 physical, unsigned width, u32& value) noexcept {
    if (physical != 0x1F801800 || width != 1) return false;
    // PRMEMPT and PRMWRDY describe an empty parameter FIFO. No response,
    // data request, command busy or ADPCM busy is asserted by this fixture.
    value = status();
    ++reads;
    return true;
}
bool CdromRegisters::write(u32 physical, unsigned width, u32 value) noexcept {
    if (physical != 0x1F801800 || width != 1) return false;
    bank_ = value & 3; // Upper status bits are read-only in every bank.
    ++writes;
    return true;
}
} // namespace opengt::guest
