#include "opengt/guest.hpp"
#include "opengt/startup_services.hpp"
#include "opengt/ps1_dma.hpp"

namespace opengt::guest {
Memory::Memory(std::uint8_t* ram, std::size_t bytes, std::uint8_t* scratch) noexcept
    : ram_(ram), bytes_((bytes == 0x200000 || bytes == 0x800000) ? bytes : 0), scratch_(scratch) {}

std::uint8_t* Memory::resolve(u32 address, unsigned width) const noexcept {
    if ((width != 1 && width != 2 && width != 4) || (address & (width - 1))) return nullptr;
    // KUSEG low physical window plus KSEG0/KSEG1. KSEG2 is not a RAM alias.
    if (address >= 0x20000000u && (address < 0x80000000u || address >= 0xC0000000u)) return nullptr;
    const u32 physical = address & 0x1FFFFFFFu;
    if (physical < 0x800000 && ram_ && bytes_) {
        const auto offset = physical & static_cast<u32>(bytes_ - 1);
        if (width <= bytes_ - offset) return ram_ + offset;
    }
    if (physical >= 0x1F800000 && physical <= 0x1F800400 - width && scratch_)
        return scratch_ + (physical - 0x1F800000);
    return nullptr;
}

bool Memory::read(u32 address, unsigned width, u32& value) const noexcept {
    if (address < 0x20000000u || (address >= 0x80000000u && address < 0xC0000000u)) {
        const auto physical = address & 0x1FFFFFFFu;
        if (irq_ && irq_->read(physical, width, value)) return true;
        if (dma_ && dma_->read(physical, width, value)) return true;
        if (dma_irq_ && dma_irq_->read(physical, width, value)) return true;
    }
    const auto* p = resolve(address, width);
    if (!p) return false;
    value = 0;
    for (unsigned i = 0; i < width; ++i) value |= static_cast<u32>(p[i]) << (8 * i);
    return true;
}
bool Memory::write(u32 address, unsigned width, u32 value) noexcept {
    if (address < 0x20000000u || (address >= 0x80000000u && address < 0xC0000000u)) {
        const auto physical = address & 0x1FFFFFFFu;
        if (irq_ && irq_->write(physical, width, value)) return true;
        if (dma_ && dma_->write(physical, width, value)) return true;
        if (dma_irq_ && dma_irq_->write(physical, width, value)) return true;
        if (timer_ && timer_->write(physical, width, value)) return true;
    }
    auto* p = resolve(address, width);
    if (!p) return false;
    for (unsigned i = 0; i < width; ++i) p[i] = static_cast<std::uint8_t>(value >> (8 * i));
    return true;
}

u32 Context::begin() noexcept {
    in_delay = next_delay;
    next_delay = false;
    write(pending_register, pending_value);
    pending_register = 0;
    ++instructions;
    const u32 sequential = next_pc;
    next_pc += 4;
    return sequential;
}
void Context::load(unsigned index, u32 value) noexcept {
    pending_register = index;
    pending_value = value;
}
void Context::fault(Stop reason, u32 address) noexcept {
    stop = reason;
    epc = in_delay ? pc - 4 : pc;
    if (reason == Stop::syscall) {
        // Exception code 8; preserve pending IRQs and leave BadVAddr alone.
        cause = (cause & ~0x8000007Cu) | (in_delay ? 0x80000000u : 0) | 0x20u;
        sr = (sr & ~0x3Fu) | ((sr << 2) & 0x3Fu);
        return;
    }
    const u32 code = reason == Stop::overflow ? 12u : reason == Stop::unmapped ? 7u :
                     reason == Stop::address_store ? 5u : 4u;
    cause = (in_delay ? 0x80000000u : 0) | (code << 2);
    if (reason != Stop::overflow) bad_vaddr = address;
    // Diagnostic stop only: exception-vector execution is a later milestone.
}

void multiply(Context& c, u32 a, u32 b, bool is_signed) noexcept {
    const std::uint64_t product = is_signed
        ? static_cast<std::uint64_t>(signed_value(a) * signed_value(b))
        : static_cast<std::uint64_t>(a) * b;
    c.lo = static_cast<u32>(product);
    c.hi = static_cast<u32>(product >> 32);
}
void divide(Context& c, u32 a, u32 b, bool is_signed) noexcept {
    if (!b) { c.hi = a; c.lo = is_signed && signed_value(a) < 0 ? 1u : ~u32{0}; return; }
    if (is_signed) {
        // 64-bit division also defines INT32_MIN / -1 without host UB.
        c.lo = static_cast<u32>(signed_value(a) / signed_value(b));
        c.hi = static_cast<u32>(signed_value(a) % signed_value(b));
    } else { c.lo = a / b; c.hi = a % b; }
}
void checked_add(Context& c, unsigned dst, u32 a, u32 b, bool subtract) noexcept {
    const auto result = subtract ? signed_value(a) - signed_value(b) : signed_value(a) + signed_value(b);
    if (result < -2147483648LL || result > 2147483647LL) c.fault(Stop::overflow);
    else c.write(dst, static_cast<u32>(result));
}
bool guest_read(Context& c, Memory& m, u32 address, unsigned width, u32& value) noexcept {
    if (address & (width - 1)) { c.fault(Stop::address_load, address); return false; }
    if (!m.read(address, width, value)) { c.fault(Stop::unmapped, address); return false; }
    return true;
}
void guest_write(Context& c, Memory& m, u32 address, unsigned width, u32 value) noexcept {
    if (address & (width - 1)) c.fault(Stop::address_store, address);
    else if (!m.write(address, width, value)) c.fault(Stop::unmapped, address);
}
void store_merge(Context& c, Memory& m, u32 address, u32 value, bool left) noexcept {
    const u32 aligned = address & ~3u, offset = address & 3u;
    u32 original = 0;
    if (!guest_read(c, m, aligned, 4, original)) return;
    // Little-endian SWL fills bytes [0,offset]; SWR fills [offset,3].
    const unsigned first = left ? 0 : offset, last = left ? offset : 3;
    for (unsigned byte = first; byte <= last; ++byte) {
        const unsigned source = left ? (3 - offset + byte) : (byte - offset);
        original = (original & ~(0xFFu << (byte * 8))) |
                   (((value >> (source * 8)) & 0xFFu) << (byte * 8));
    }
    guest_write(c, m, aligned, 4, original);
}
const char* stop_name(Stop stop) noexcept {
    switch (stop) {
    case Stop::running: return "running";
    case Stop::returned: return "returned";
    case Stop::budget: return "budget";
    case Stop::address_load: return "load alignment";
    case Stop::address_store: return "store alignment";
    case Stop::unmapped: return "unmapped memory";
    case Stop::overflow: return "overflow";
    case Stop::unknown_pc: return "unknown PC";
    case Stop::delay_control: return "delay control";
    case Stop::bios: return "unresolved BIOS";
    case Stop::syscall: return "unresolved syscall";
    case Stop::interrupt: return "unresolved IRQ delivery";
    }
    return "unknown";
}
} // namespace opengt::guest
