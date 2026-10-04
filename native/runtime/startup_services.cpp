#include "opengt/startup_services.hpp"

namespace opengt::guest {
namespace {
u32 return_status(u32 saved) noexcept {
    // R3000 RFE restores the current/previous KU/IE pairs, retaining old pair.
    return (saved & ~0xFu) | ((saved >> 2) & 0xFu);
}
}
bool StartupBios::dispatch_syscall(Context& c) noexcept {
    if (c.stop != Stop::syscall || c.in_delay || c.next_delay || c.pending_register ||
        (c.read(4) != 1 && c.read(4) != 2)) return false;
    if (c.read(4) == 1) {
        c.write(2, (c.sr & 0x404) == 0x404 ? 1 : 0);
        c.sr &= ~u32{0x404};
        ++critical_entries;
    } else {
        c.sr |= 0x404;
        ++critical_exits;
    }
    c.sr = return_status(c.sr);
    c.pc = c.epc + 4;
    c.next_pc = c.pc + 4;
    c.stop = Stop::running;
    ++syscall_calls;
    return true;
}
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
    if (c.stop != Stop::running || c.next_delay || c.pending_register) return false;
    if (c.pc == 0xA0 && (c.read(9) == 0x72 || c.read(9) == 0x56)) {
        // _96_remove enters critical state, closes ACK/DNE/RDY/END/ERR and
        // attempts handler dequeue, without leaving the critical section.
        // Model only this lifecycle projection; retail dequeue is bugged.
        const u32 saved = (c.sr & ~0x3Fu) | ((c.sr << 2) & 0x3Fu);
        c.sr = return_status(saved & ~u32{0x404});
        ++critical_entries;
        cd_events_open = 0;
        cd_close_attempts += 5;
        ++cd_dequeue_attempts;
        cd_dequeue_unresolved = true;
        ++cd_remove_calls;
    } else if (c.pc == 0xB0 && c.read(9) == 0x19) {
        hook_buffer = c.read(4);
        // Matches BiosB's IntrEnvInInterruptAddr projection, retained for a future
        // callback dispatcher. Only registration runs here; the guest setjmp ran
        // natively and produced the saved register block itself.
        interrupt_environment = hook_buffer ? hook_buffer - 0x36 : 0;
    } else if (c.pc == 0xB0 && c.read(9) == 0x5B) {
        // ChangeClearPAD: raw flag exchange, matching OpenBIOS setSIO0AutoAck.
        // PsyQ declares this void; retain the BIOS old-flag result in v0.
        const u32 flag = c.read(4);
        c.write(2, pad_auto_ack);
        pad_auto_ack = flag;
        ++pad_calls;
    } else if (c.pc == 0xC0 && c.read(9) == 0x0A && c.read(4) < 4) {
        // ChangeClearRCnt: 0..2 hardware timers; 3 is the BIOS VBlank handler.
        // This controls handler policy, not timer mode registers or I_MASK.
        const unsigned timer = c.read(4);
        c.write(2, timer_auto_ack[timer]);
        timer_auto_ack[timer] = c.read(5);
        ++rcnt_calls;
    } else return false;
    ++calls;
    c.pc = c.read(31);
    c.next_pc = c.pc + 4;
    c.in_delay = false;
    return true;
}
bool StartupBios::acknowledge_pad_vblank(InterruptController& irq) const noexcept {
    if (!pad_auto_ack) return false;
    // IRQ0 VBlank, NOT IRQ7 (the separate serial controller interrupt).
    return irq.write(0x1F801070, 4, ~u32{1});
}
} // namespace opengt::guest
