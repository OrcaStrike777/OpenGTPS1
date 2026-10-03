#pragma once
#include <cstddef>
#include <cstdint>
#include "opengt/ps1_interrupts.hpp"

namespace opengt::guest {
struct TimerSetup;
class DmaInterrupts;
using u32 = std::uint32_t;
enum class Stop : u32 { running, returned, budget, address_load, address_store,
                        unmapped, overflow, unknown_pc, delay_control, bios };


// Caller owns storage; only retail 2 MiB or explicit devkit 8 MiB is accepted.
// Devices, BIOS and caches are not silently backed by anonymous RAM.
class Memory {
public:
    Memory(std::uint8_t* ram, std::size_t bytes, std::uint8_t* scratch) noexcept;
    void attach_interrupts(InterruptController* irq) noexcept { irq_ = irq; }
    void attach_dma_priority(DmaPriority* dma) noexcept { dma_ = dma; }
    void attach_dma_interrupts(DmaInterrupts* dma) noexcept { dma_irq_ = dma; }
    void attach_timer_setup(TimerSetup* timer) noexcept { timer_ = timer; }
    bool read(u32 address, unsigned width, u32& value) const noexcept;
    bool write(u32 address, unsigned width, u32 value) noexcept;
private:
    std::uint8_t* resolve(u32 address, unsigned width) const noexcept;
    std::uint8_t* ram_;
    std::size_t bytes_;
    std::uint8_t* scratch_;
    InterruptController* irq_{};
    DmaPriority* dma_{};
    DmaInterrupts* dma_irq_{};
    TimerSetup* timer_{};
};

struct Context {
    u32 hi{}, lo{}, pc{}, next_pc{};
    u32 bad_vaddr{}, sr{}, cause{}, epc{}, prid{2};
    u32 instructions{}, pending_register{}, pending_value{};
    bool next_delay{}, in_delay{};
    Stop stop{Stop::running};
    u32 read(unsigned index) const noexcept { return index == 0 ? 0 : gpr_[index]; }
    void write(unsigned index, u32 value) noexcept { if (index != 0) gpr_[index] = value; }
    void start(u32 entry) noexcept { pc = entry; next_pc = entry + 4; }
    // Emitters snapshot source operands BEFORE retiring the previous load.
    u32 begin() noexcept;
    void load(unsigned index, u32 value) noexcept;
    void branch(u32 target) noexcept { next_pc = target; next_delay = true; }
    void fault(Stop reason, u32 address = 0) noexcept;
private:
    u32 gpr_[32]{};
};

// Defined bit arithmetic, independent of host signed-shift/overflow behavior.
inline std::int64_t signed_value(u32 v) noexcept {
    return (v & 0x80000000u) ? static_cast<std::int64_t>(v) - 0x100000000LL : v;
}
inline u32 arithmetic_shift(u32 v, u32 n) noexcept {
    n &= 31;
    return n == 0 ? v : (v >> n) | ((v & 0x80000000u) ? (~u32{0} << (32 - n)) : 0);
}
void multiply(Context& c, u32 a, u32 b, bool is_signed) noexcept;
void divide(Context& c, u32 a, u32 b, bool is_signed) noexcept;
void checked_add(Context& c, unsigned dst, u32 a, u32 b, bool subtract) noexcept;
bool guest_read(Context& c, Memory& m, u32 address, unsigned width, u32& value) noexcept;
void guest_write(Context& c, Memory& m, u32 address, unsigned width, u32 value) noexcept;
void store_merge(Context& c, Memory& m, u32 address, u32 value, bool left) noexcept;
const char* stop_name(Stop stop) noexcept;
} // namespace opengt::guest
