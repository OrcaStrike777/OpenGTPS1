#pragma once
#include <cstdint>

namespace opengt::guest {
struct Context;

// PS1 IRQ0..IRQ10: status is an edge latch; the mask only gates CPU IRQ2.
// No device clock or callback scheduling is hidden inside this controller.
class InterruptController {
public:
    static constexpr std::uint32_t bits = 0x7FF;
    void reset() noexcept;
    void set_line(unsigned irq, bool asserted) noexcept;
    void pulse(unsigned irq) noexcept;
    bool read(std::uint32_t physical, unsigned width, std::uint32_t& value) noexcept;
    bool write(std::uint32_t physical, unsigned width, std::uint32_t value) noexcept;
    void sync_cpu(Context&) const noexcept;
    std::uint32_t pending() const noexcept { return pending_; }
    std::uint32_t mask() const noexcept { return mask_; }
    bool requested() const noexcept { return (pending_ & mask_) != 0; }
    std::uint32_t stat_reads{}, stat_writes{}, mask_reads{}, mask_writes{};
private:
    std::uint32_t pending_{}, mask_{}, lines_{};
};

// Fixed NTSC non-interlaced loader fixture: 263 lines * 2152.5 CPU cycles.
// Half-cycle phase retains the fractional field period without host time.
// Caller supplies elapsed emulated cycles; GPU mode changes remain unsupported.
struct VBlankClock {
    static constexpr std::uint32_t field_half_cycles = 1132215;
    std::uint32_t phase{}, edges{};
    void advance(std::uint32_t cpu_cycles, InterruptController&) noexcept;
};

// Only the encountered DMA priority-control latch. Channels/transfers remain
// unmapped. Mirrors PSMemory's DPCR storage, with the PS1 power-on value.
struct DmaPriority {
    std::uint32_t value{0x07654321}, reads{}, writes{};
    bool read(std::uint32_t physical, unsigned width, std::uint32_t& output) noexcept;
    bool write(std::uint32_t physical, unsigned width, std::uint32_t input) noexcept;
};
} // namespace opengt::guest
