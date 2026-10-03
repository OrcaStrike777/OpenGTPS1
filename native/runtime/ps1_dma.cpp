#include "opengt/ps1_dma.hpp"

namespace opengt::guest {
namespace {
constexpr std::uint32_t base = 0x1F8010F4;
constexpr std::uint32_t controls = 0x00FF807F; // bits 7..14 unused
constexpr std::uint32_t flags = 0x7F000000;
bool access(std::uint32_t address, unsigned width) noexcept {
    return (width == 1 || width == 2 || width == 4) && !(address & (width - 1)) &&
           address >= base && address <= base + 4 - width;
}
}
void DmaInterrupts::reset() noexcept {
    state_ = reads = writes = completions = irq_rises = channel_reads = channel_writes = 0;
    irq_.set_line(3, false); // I_STAT's latched request has its own acknowledgement
}
void DmaInterrupts::update_irq() noexcept {
    const bool was_active = (state_ & 0x80000000u) != 0;
    // Channel enables gate completion latching, not already-latched flags.
    const bool active = (state_ & 0x8000) || ((state_ & 0x800000) && (state_ & flags));
    state_ = (state_ & 0x7FFFFFFF) | (active ? 0x80000000u : 0);
    if (active && !was_active) ++irq_rises;
    irq_.set_line(3, active);
}
bool DmaInterrupts::read(std::uint32_t physical, unsigned width, std::uint32_t& output) noexcept {
    if (physical >= 0x1F801080 && physical < 0x1F8010F0) ++channel_reads;
    if (!access(physical, width)) return false;
    ++reads;
    output = state_ >> ((physical - base) * 8);
    if (width < 4) output &= (1u << (width * 8)) - 1;
    return true;
}
bool DmaInterrupts::write(std::uint32_t physical, unsigned width, std::uint32_t input) noexcept {
    if (physical >= 0x1F801080 && physical < 0x1F8010F0) ++channel_writes;
    if (!access(physical, width)) return false;
    ++writes;
    // On-die PS1 MMIO latches the full shifted source bus word on SB/SH too.
    // Do not merge with readback: doing so would acknowledge unrelated flags.
    input <<= (physical - base) * 8;
    state_ = (state_ & 0x80000000u) | (state_ & flags & ~input) | (input & controls);
    update_irq();
    return true;
}
void DmaInterrupts::complete(unsigned channel) noexcept {
    if (channel >= 7) return;
    ++completions;
    if ((state_ & 0x800000) && (state_ & (1u << (16 + channel))))
        state_ |= 1u << (24 + channel);
    update_irq();
}
} // namespace opengt::guest
