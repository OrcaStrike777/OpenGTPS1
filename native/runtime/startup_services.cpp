#include "opengt/startup_services.hpp"

namespace opengt::guest {
bool TimerSetup::write(u32 physical, unsigned width, u32 value) noexcept {
    if ((width != 2 && width != 4) || physical < 0x1F801104 || physical > 0x1F801124 ||
        ((physical - 0x1F801104) & 0xF)) return false;
    const unsigned timer = (physical - 0x1F801104) / 0x10;
    // Writable timer configuration bits 0..9. Writing mode resets the counter;
    // status/IRQ/readback and elapsed ticks belong to the future timer device.
    mode[timer] = value & 0x3FF;
    counter[timer] = 0;
    ++writes;
    return true;
}
bool StartupBios::dispatch(Context& c) noexcept {
    if (c.pc != 0xB0 || c.read(9) != 0x19 || c.next_delay || c.pending_register) return false;
    hook_buffer = c.read(4);
    // Matches BiosB's IntrEnvInInterruptAddr projection, retained for a future
    // callback dispatcher. Only registration runs here; the guest setjmp ran
    // natively and produced the saved register block itself.
    interrupt_environment = hook_buffer ? hook_buffer - 0x36 : 0;
    ++calls;
    c.pc = c.read(31);
    c.next_pc = c.pc + 4;
    c.in_delay = false;
    return true;
}
} // namespace opengt::guest
