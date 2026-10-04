#include "opengt/startup_services.hpp"

namespace opengt::guest {
namespace {
u32 return_status(u32 saved) noexcept {
    // R3000 RFE restores the current/previous KU/IE pairs, retaining old pair.
    return (saved & ~0xFu) | ((saved >> 2) & 0xFu);
}
}
bool StartupBios::dispatch_console(Context& c, const Memory& m) noexcept {
    const bool formatted = c.pc == 0xA0 && c.read(9) == 0x3F;
    if (c.stop != Stop::running || c.next_delay || c.pending_register ||
        !(formatted || (c.pc == 0xA0 && c.read(9) == 0x3E) || (c.pc == 0xB0 && c.read(9) == 0x3F)))
        return false;
    // Stage one complete call: failures cannot partially emit/replay output.
    char staged[console_capacity]{};
    u32 size = 0, column = console_column;
    const auto emit = [&](char ch) {
        if (console_size > console_capacity || size >= console_capacity - console_size) return false;
        staged[size++] = ch;
        return true;
    };
    const auto put = [&](u32 ch) {
        if (ch == '\t') {
            const u32 spaces = 8 - (column & 7);
            for (u32 n = 0; n < spaces; ++n) if (!emit(' ')) return false;
            column += spaces;
        } else if (ch == '\n') {
            if (!emit('\r') || !emit('\n')) return false;
            column = 0;
        } else {
            if (!emit(static_cast<char>(ch))) return false;
            if (ch == '\r') column = 0;
            else if (ch == '\b') { if (column) --column; }
            else if (ch >= 0x20 && ch != 0x7F) ++column;
        }
        return true;
    };
    const auto read = [&](u32 address, unsigned width, u32& value) {
        const u32 physical = address & 0x1FFFFFFF;
        // Format/argument reads must not touch side-effecting MMIO.
        return (physical <= 0x800000 - width ||
                (physical >= 0x1F800000 && physical <= 0x1F800400 - width)) &&
               m.read(address, width, value);
    };
    const u32 address = c.read(4);
    if (formatted && !address) return false; // printf null format is unmodeled.
    u32 offset = 0, argument = 0;
    const auto next = [&](u32& ch) {
        if (offset > console_capacity || address > ~u32{0} - offset) return false;
        if (!address) {
            constexpr char null_text[] = "<NULL>";
            ch = static_cast<unsigned char>(null_text[offset++]);
            return true;
        }
        return read(address + offset++, 1, ch);
    };
    for (;;) {
        u32 ch = 0;
        if (!next(ch)) return false;
        if (!ch) break;
        if (!formatted || ch != '%') {
            if (!put(ch)) return false;
            continue;
        }
        if (!next(ch)) return false;
        if (ch == '%') { if (!put('%')) return false; continue; }
        const bool zero_pad = ch == '0';
        if (zero_pad && !next(ch)) return false;
        u32 width = 0;
        while (ch >= '0' && ch <= '9') {
            width = width * 10 + ch - '0';
            if (width > console_capacity || !next(ch)) return false;
        }
        // Deliberately bounded printf subset: hexadecimal with field width.
        // Other conversions/flags/precision/lengths stop without side effects.
        if (ch != 'x' && ch != 'X') return false;
        u32 value = 0;
        if (argument < 3) value = c.read(5 + argument);
        else {
            const u32 stack_offset = 16 + (argument - 3) * 4;
            if (c.read(29) > ~u32{0} - stack_offset ||
                !read(c.read(29) + stack_offset, 4, value)) return false;
        }
        ++argument;
        char digits[8];
        unsigned count = 0;
        do {
            const u32 digit = value & 15;
            digits[count++] = static_cast<char>(digit < 10 ? '0' + digit :
                                               (ch == 'x' ? 'a' : 'A') + digit - 10);
            value >>= 4;
        } while (value);
        for (u32 n = count; n < width; ++n) if (!put(zero_pad ? '0' : ' ')) return false;
        while (count) if (!put(digits[--count])) return false;
    }
    for (u32 i = 0; i < size; ++i) console[console_size + i] = staged[i];
    console_size += size; console[console_size] = 0; console_column = column;
    if (formatted) ++printf_calls; else ++puts_calls;
    ++calls;
    // These BIOS console APIs specify no result register. Preserve it rather
    // than imposing host libc's count. Return only after retaining all output.
    c.start(c.read(31)); c.in_delay = false;
    return true;
}
bool StartupBios::dispatch_interrupt(Context& c, Memory& m, InterruptController& irq) noexcept {
    if (c.stop != Stop::running || !(c.sr & 1) || !(c.sr & c.cause & 0xFF00)) return false;
    // Defer until a complete branch/load boundary; never discard a delay slot
    // or replay a branch whose link register may already have been written.
    if (c.next_delay || c.pending_register) { ++irq_deferred; return false; }
    // No general BIOS event/handler chain yet. Refuse unsupported policies,
    // other sources, and explicit re-enabling within our single saved frame.
    if (irq_active || (c.cause & c.sr & 0xFF00) != 0x400 ||
        (irq.pending() & irq.mask()) != 1 || pad_auto_ack || timer_auto_ack[3] || !hook_buffer) {
        c.stop = Stop::interrupt; return false;
    }
    // HookEntryInt is a guest setjmp block, not a direct callback address.
    // Validate/read the entire block before changing context or consuming IRQ.
    u32 hook[12]{};
    if (hook_buffer > 0xFFFFFFD0u) { c.stop = Stop::interrupt; return false; }
    for (unsigned i = 0; i < 12; ++i)
        if (!m.read(hook_buffer + i * 4, 4, hook[i])) { c.stop = Stop::interrupt; return false; }
    if (!hook[0] || (hook[0] & 3) || (hook[1] & 3)) { c.stop = Stop::interrupt; return false; }
    interrupted_ = c;
    interrupted_.sr = (c.sr & ~0x3Fu) | ((c.sr << 2) & 0x3Fu);
    interrupted_.cause = c.cause & ~0x8000007Cu;
    interrupted_.epc = c.pc;
    interrupted_.in_delay = false;
    irq_resume_pc = c.pc;
    irq_hook_pc = hook[0];
    c.sr = interrupted_.sr; c.cause = interrupted_.cause; c.epc = c.pc;
    c.write(31, hook[0]); c.write(29, hook[1]); c.write(30, hook[2]);
    for (unsigned i = 0; i < 8; ++i) c.write(16 + i, hook[3 + i]);
    c.write(28, hook[11]); c.write(2, 1);
    c.start(hook[0]); c.in_delay = false;
    irq_active = true; ++irq_entries;
    return true;
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
    if (c.pc == 0xB0 && c.read(9) == 0x17) {
        if (!irq_active) return false;
        const u32 instructions = c.instructions;
        c = interrupted_;
        c.instructions = instructions;
        c.sr = return_status(c.sr);
        irq_active = false; ++irq_returns; ++calls;
        // Caller resynchronizes Cause.IP2 from the live controller next step.
        // Return to interrupted PC, never this BIOS wrapper's RA.
        return true;
    } else if (c.pc == 0xA0 && (c.read(9) == 0x72 || c.read(9) == 0x56)) {
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
        // Retain the desktop IntrEnvInInterruptAddr projection for diagnostics.
        // Native IRQ delivery uses the actual guest setjmp block instead of
        // bypassing the original handler via this SDK-specific offset.
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
