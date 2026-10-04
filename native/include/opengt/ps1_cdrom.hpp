#pragma once
#include "opengt/guest.hpp"

namespace opengt::guest {
// Bounded idle CD controller: empty parameter/result/data FIFOs, no command
// or ADPCM work. Index/status, flag read/ack and idle Request disable only.
class CdromRegisters {
public:
    bool read(u32 physical, unsigned width, u32& value) noexcept;
    bool write(u32 physical, unsigned width, u32 value) noexcept;
    u32 bank() const noexcept { return bank_; }
    u32 status() const noexcept { return 0x18u | bank_; }
    // HC05/decoder flag latch. No command engine sets it yet; synthetic tests
    // seed pending flags to verify selective acknowledgement (not completions).
    u32 interrupt_flags{};
    u32 flag_reads{}, acknowledgements{};
    u32 request_writes{};
    u32 reads{}, writes{};
private:
    u32 bank_{};
};
} // namespace opengt::guest
