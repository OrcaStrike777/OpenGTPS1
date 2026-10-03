#pragma once
#include "opengt/ps1_interrupts.hpp"

namespace opengt::guest {
// DICR only. The caller owns IRQ state and must outlive this controller.
// No transfer buffers, channel engines, host callbacks or device clocks.
class DmaInterrupts {
public:
    explicit DmaInterrupts(InterruptController& irq) noexcept : irq_(irq) {}
    void reset() noexcept;
    bool read(std::uint32_t physical, unsigned width, std::uint32_t& output) noexcept;
    bool write(std::uint32_t physical, unsigned width, std::uint32_t input) noexcept;
    // Future transfer engines call this at completion, not at CHCR start.
    void complete(unsigned channel) noexcept;
    std::uint32_t state() const noexcept { return state_; }
    std::uint32_t reads{}, writes{}, completions{}, irq_rises{};
    // Attempts at still-unmapped MADR/BCR/CHCR registers, for bounded diagnostics.
    std::uint32_t channel_reads{}, channel_writes{};
private:
    void update_irq() noexcept;
    InterruptController& irq_;
    std::uint32_t state_{};
};
} // namespace opengt::guest
