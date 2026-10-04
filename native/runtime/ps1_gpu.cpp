#include "opengt/ps1_gpu.hpp"

namespace opengt::guest {
u32 GpuControl::status() const noexcept {
    // Reset GP1 status: noninterlaced field bit, display disabled, idle command
    // input and empty write FIFO. DMA off, no read data and no GPU IRQ.
    // These ready bits describe an empty device, never a completed transfer.
    const u32 line = (clock_.phase / 4305 + 256) % 263;
    const bool odd = line >= 16 && line < 256 && (line & 1);
    return 0x14002000u | (display_disabled_ ? 0x00800000u : 0u) |
           (odd ? 0x80000000u : 0u);
}
bool GpuControl::read(u32 physical, unsigned width, u32& value) noexcept {
    if (physical != 0x1F801814 || width != 4) return false;
    value = status();
    ++reads;
    return true;
}
bool GpuControl::write(u32 physical, unsigned width, u32 value) noexcept {
    if (physical != 0x1F801814 || width != 4) return false;
    // GP1 decodes six command bits; 40h..FFh mirror 00h..3Fh.
    if (((value >> 24) & 0x3F) != 3) return false;
    display_disabled_ = (value & 1) != 0;
    last_command = value;
    ++writes;
    return true;
}
} // namespace opengt::guest
