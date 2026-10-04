#pragma once
#include "opengt/guest.hpp"

namespace opengt::guest {
// Bounded idle CD controller: empty parameter/result/data FIFOs, no command
// or ADPCM work. Only ADDRESS/HSTS is implemented; banked ports still trap.
class CdromRegisters {
public:
    bool read(u32 physical, unsigned width, u32& value) noexcept;
    bool write(u32 physical, unsigned width, u32 value) noexcept;
    u32 bank() const noexcept { return bank_; }
    u32 status() const noexcept { return 0x18u | bank_; }
    u32 reads{}, writes{};
private:
    u32 bank_{};
};
} // namespace opengt::guest
