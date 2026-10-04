#include "opengt/ps1_interrupts.hpp"
#include "opengt/guest.hpp"

namespace opengt::guest {
namespace {
bool register_access(std::uint32_t address, unsigned width, std::uint32_t base) noexcept {
    return (address == base && (width == 2 || width == 4)) ||
           (address == base + 2 && width == 2);
}
}
void InterruptController::reset() noexcept { *this = InterruptController{}; }
void InterruptController::set_line(unsigned irq, bool asserted) noexcept {
    if (irq > 10) return;
    const auto bit = std::uint32_t{1} << irq;
    if (asserted) { pending_ |= bit & ~lines_; lines_ |= bit; }
    else lines_ &= ~bit;
}
void InterruptController::pulse(unsigned irq) noexcept {
    if (irq <= 10) pending_ |= std::uint32_t{1} << irq;
}
bool InterruptController::read(std::uint32_t address, unsigned width, std::uint32_t& value) noexcept {
    if (register_access(address, width, 0x1F801070)) {
        ++stat_reads; value = address == 0x1F801070 ? pending_ : 0; return true;
    }
    if (register_access(address, width, 0x1F801074)) {
        ++mask_reads; value = address == 0x1F801074 ? mask_ : 0; return true;
    }
    return false;
}
bool InterruptController::write(std::uint32_t address, unsigned width, std::uint32_t value) noexcept {
    if (register_access(address, width, 0x1F801070)) {
        ++stat_writes;
        if (address == 0x1F801070) pending_ &= value & bits; // zero clears, one preserves
        return true;
    }
    if (register_access(address, width, 0x1F801074)) {
        ++mask_writes;
        if (address == 0x1F801074) mask_ = value & bits;
        return true;
    }
    return false;
}
void InterruptController::sync_cpu(Context& c) const noexcept {
    c.cause = (c.cause & ~0x400u) | (requested() ? 0x400u : 0u);
}
void VBlankClock::advance(std::uint32_t cycles, InterruptController& irq) noexcept {
    const std::uint64_t total = phase + std::uint64_t{cycles} * 2;
    if (total < field_half_cycles) { phase = static_cast<std::uint32_t>(total); return; }
    edges += static_cast<std::uint32_t>(total / field_half_cycles);
    phase = static_cast<std::uint32_t>(total % field_half_cycles);
    // I_STAT coalesces multiple unacknowledged edges, independently of I_MASK.
    irq.pulse(0);
}
bool DmaPriority::read(std::uint32_t address, unsigned width, std::uint32_t& output) noexcept {
    if (!register_access(address, width, 0x1F8010F0)) return false;
    ++reads;
    output = width == 4 ? value : (value >> ((address & 2) * 8)) & 0xFFFF;
    return true;
}
bool DmaPriority::write(std::uint32_t address, unsigned width, std::uint32_t input) noexcept {
    if (!register_access(address, width, 0x1F8010F0)) return false;
    ++writes;
    if (width == 4) value = input;
    else {
        const auto shift = (address & 2) * 8;
        value = (value & ~(0xFFFFu << shift)) | ((input & 0xFFFF) << shift);
    }
    return true;
}
} // namespace opengt::guest
