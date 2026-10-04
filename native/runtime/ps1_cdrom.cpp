#include "opengt/ps1_cdrom.hpp"

namespace opengt::guest {
bool CdromRegisters::read(u32 physical, unsigned width, u32& value) noexcept {
    if (width != 1) return false;
    if (physical == 0x1F801803 && (bank_ & 1)) {
        value = 0xE0u | (interrupt_flags & 0x1F); // HINTSTS, banks 1 and 3.
        ++flag_reads;
        ++reads;
        return true;
    }
    if (physical != 0x1F801800) return false;
    // PRMEMPT and PRMWRDY describe an empty parameter FIFO. No response,
    // data request, command busy or ADPCM busy is asserted by this fixture.
    value = status();
    ++reads;
    return true;
}
bool CdromRegisters::write(u32 physical, unsigned width, u32 value) noexcept {
    if (width != 1) return false;
    if (physical == 0x1F801803 && bank_ == 1) {
        // HCLRCTL: low five bits are W1C, including the encoded HC05 type.
        // Decoder reset / XA buffer clear / parameter FIFO clear are outside
        // this bounded model. Reject them before changing any flag or counter.
        if (value & 0xE0) return false;
        interrupt_flags &= ~(value & 0x1F);
        ++acknowledgements;
        ++writes;
        return true;
    }
    if (physical != 0x1F801800) return false;
    bank_ = value & 3; // Upper status bits are read-only in every bank.
    ++writes;
    return true;
}
} // namespace opengt::guest
