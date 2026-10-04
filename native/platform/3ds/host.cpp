#include "host.hpp"
#include "opengt/guest_tests.hpp"
#include "opengt/boot_probe.hpp"

#include <cstdio>

namespace opengt::ctr {
namespace {
std::uint32_t buttons(u32 keys) noexcept {
    const struct { u32 key; platform::Button button; } mapping[] = {
        {KEY_A, platform::a}, {KEY_B, platform::b},
        {KEY_X, platform::x}, {KEY_Y, platform::y},
        {KEY_DUP, platform::up}, {KEY_DDOWN, platform::down},
        {KEY_DLEFT, platform::left}, {KEY_DRIGHT, platform::right},
        {KEY_L, platform::l}, {KEY_R, platform::r},
        {KEY_START, platform::start}, {KEY_SELECT, platform::select},
    };
    std::uint32_t result = 0;
    for (const auto& item : mapping) if (keys & item.key) result |= item.button;
    return result;
}

float axis(s16 value) noexcept {
    const float normalized = static_cast<float>(value) / 156.0f;
    return normalized < -1.0f ? -1.0f : normalized > 1.0f ? 1.0f : normalized;
}
}

bool Host::initialize() noexcept {
    if (gfx_ready_) return false; // one initialization per host
    gfxInitDefault();
    gfx_ready_ = true;
    gfxSet3D(false);
    consoleInit(GFX_BOTTOM, nullptr);
    std::printf("OpenGTPS1 / Old 3DS bootstrap\n");
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        std::printf("Citro3D initialization failed.\n");
        return false;
    }
    c3d_ready_ = true;
    // 3DS framebuffers are rotated. Single mono top target; no AA or depth yet.
    top_ = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, -1);
    if (!top_) {
        std::printf("Top render target allocation failed.\n");
        return false;
    }
    constexpr u32 transfer = GX_TRANSFER_FLIP_VERT(0) |
        GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
        GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
        GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
        GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);
    C3D_RenderTargetSetOutput(top_, GFX_TOP, GFX_LEFT, transfer);
    return true;
}

Host::~Host() {
    // Deleting the target waits for pending transfers before releasing VRAM.
    if (top_) C3D_RenderTargetDelete(top_);
    if (c3d_ready_) C3D_Fini();
    if (gfx_ready_) gfxExit();
}

bool Host::running() noexcept { return aptMainLoop(); }

platform::InputState Host::poll() noexcept {
    hidScanInput();
    circlePosition circle{};
    hidCircleRead(&circle);
    return {buttons(hidKeysHeld()), buttons(hidKeysDown()), buttons(hidKeysUp()),
            axis(circle.dx), axis(circle.dy)};
}

std::uint64_t Host::ticks() const noexcept { return svcGetSystemTick(); }
std::uint64_t Host::ticks_per_second() const noexcept { return SYSCLOCK_ARM11; }
void Host::wait_vblank() noexcept { gspWaitForVBlank(); }
std::size_t Host::linear_free_bytes() const noexcept { return linearSpaceFree(); }

bool Host::present(const platform::Diagnostics& d) noexcept {
    if (!top_ || !C3D_FrameBegin(0)) return false;
    // A visibly different clear while A is held proves input reaches graphics.
    const u32 color = (d.input.held & platform::a) ? 0x287A48FF : 0x183048FF;
    C3D_RenderTargetClear(top_, C3D_CLEAR_COLOR, color, 0);
    C3D_FrameDrawOn(top_);
    C3D_FrameEnd(0);
    // Bottom console owns its framebuffer; Citro3D owns top-screen swaps.
    // Do not call gfxSwapBuffers or a second VBlank wait here.
    const bool boot_page = d.boot_page && d.boot && d.boot->available;
    if (boot_page != previous_boot_page_) consoleClear();
    previous_boot_page_ = boot_page;
    if (boot_page) {
        const auto& b = *d.boot;
        const auto hex = [](std::uint32_t v) { return static_cast<unsigned long>(v); };
        char boundary[32]{};
        char tty_preview[25]{};
        for (unsigned i = 0; i < b.console_size && i + 1 < sizeof(tty_preview); ++i) {
            const auto ch = static_cast<unsigned char>(b.console[i]);
            tty_preview[i] = ch >= 0x20 && ch <= 0x7E ? static_cast<char>(ch) : '.';
        }
        if (b.stop == guest::Stop::bios)
            std::snprintf(boundary, sizeof(boundary), "BIOS %c(%02lx)%s",
                          b.pc == 0xA0 ? 'A' : b.pc == 0xB0 ? 'B' : 'C', hex(b.bios_api),
                          b.bios_api == 0x3F ? (b.pc == 0xA0 ? " printf" : " puts") : "");
        else if (b.stop == guest::Stop::unmapped && b.unresolved == 0x1F801803 && b.cd_bank == 0)
            std::snprintf(boundary, sizeof(boundary), "CD-ROM request (bank0)");
        else if (b.waiting_vblank)
            std::snprintf(boundary, sizeof(boundary), "VBlank IRQ/callback wait");
        else if (b.stop == guest::Stop::syscall)
            std::snprintf(boundary, sizeof(boundary), "SYS(%02lx)", hex(b.syscall_api));
        else if (b.stop == guest::Stop::unmapped && b.unresolved == 0x1F801814)
            std::snprintf(boundary, sizeof(boundary), "GPU GP1 display control");
        else std::snprintf(boundary, sizeof(boundary), "%s",
                          b.stop == guest::Stop::unmapped && b.unresolved == 0x1F8010F4 ?
                          "DICR (DMA IRQ control)" : "unexpected stop");
        std::printf("\x1b[1;1HOpenGTPS1 / Old 3DS CD probe\n"
                    "START exit / X rerun / A color\n"
                    "Y: synthetic test details\n"
                    "MIPS %u/%u hash %08lx\n"
                    "GT2 hash %u/%u sig %08lx\n"
                    "Boot runtime %u/%u / IRQ %u/%u\n"
                    "BIOS %u/%u DMA %u/%u / handled: %lu\n"
                    "GT2 BOOT %-4s (bounded stop)\n"
                    "Entry       %08lx\nPC          %08lx\n"
                    "Last OK     %08lx\nLast entry  %08lx\n"
                    "Guest entries %lu\nInstructions %lu\n"
                    "I_STAT R:%lu W:%lu\nI_MASK R:%lu W:%lu\n"
                    "IRQ pending:%03lx mask:%03lx\n"
                    "GP1 W:%lu status:%08lx GPU %u/%u\n"
                    "VBL edges:%lu IRQ enter:%lu ret:%lu\n"
                    "Callback:%s / IRQ active:%u SR:%03lx\n"
                    "CD %u/%u bank:%lu R:%lu W:%lu\n"
                    "VBL count:%lu/4 SDK count:%lu\n"
                    "Unresolved  %08lx\n"
                    "Boundary: %-24s\n"
                    "Trap: %-22s\n"
                    "BSS %s / DPCR W:%lu / Timer W:%lu\n"
                    "TTY %.23s P:%lu F:%lu\n",
                    d.guest_tests->passed, d.guest_tests->count, hex(d.guest_tests->signature),
                    d.gt2_probe->passed, d.gt2_probe->count, hex(d.gt2_probe->signature),
                    d.boot_operations->passed, d.boot_operations->count,
                    d.interrupt_tests->passed, d.interrupt_tests->count,
                    d.bios_tests->passed, d.bios_tests->count,
                    d.dma_tests->passed, d.dma_tests->count, hex(b.bios_calls),
                    b.passed ? "PASS" : "FAIL", hex(b.entry), hex(b.pc), hex(b.last_pc),
                    hex(b.last_function), hex(b.functions), hex(b.instructions),
                    hex(b.stat_reads), hex(b.stat_writes), hex(b.mask_reads), hex(b.mask_writes),
                    hex(b.irq_pending), hex(b.irq_mask), hex(b.gpu_writes), hex(b.gpu_status),
                    d.gpu_tests->passed, d.gpu_tests->count,
                    hex(b.vblank_edges), hex(b.irq_entries), hex(b.irq_returns),
                    b.entered_vblank_callback ? "YES" : "NO", static_cast<unsigned>(b.irq_active), hex(b.sr),
                    d.cdrom_tests->passed, d.cdrom_tests->count, hex(b.cd_bank), hex(b.cd_reads), hex(b.cd_writes),
                    hex(b.vblank_counter), hex(b.sdk_vblank_counter), hex(b.unresolved),
                    boundary,
                    guest::stop_name(b.stop), b.clears_verified ? "PASS" : "FAIL",
                    hex(b.dma_writes), hex(b.timer_writes), tty_preview, hex(b.puts_calls), hex(b.printf_calls));
        const unsigned first = b.trace_count > 2 ? b.trace_count - 2 : 0;
        for (unsigned i = first; i < b.trace_count; ++i)
            std::printf(" %2u: %08lx\n", i + 1, hex(b.trace[i]));
        gfxFlushBuffers();
        return true;
    }
    std::printf("\x1b[1;1HOpenGTPS1 / Old 3DS bootstrap\n"
                "Native MIPS execution diagnostics\n"
                "START exit / X rerun / A color\n"
                "Frame: %-10llu %8.2f ms\n"
                "Held: %08lx  Down: %08lx\nUp:   %08lx\n"
                "Circle: %+5.2f %+5.2f\n"
                "Linear free: %8lu KiB\nSD probe: %-16s\n"
                "Y: boot diagnostics\n",
                static_cast<unsigned long long>(d.frame), d.frame_ms,
                static_cast<unsigned long>(d.input.held),
                static_cast<unsigned long>(d.input.pressed),
                static_cast<unsigned long>(d.input.released),
                static_cast<double>(d.input.circle_x), static_cast<double>(d.input.circle_y),
                static_cast<unsigned long>(d.linear_free_bytes / 1024),
                platform::status_name(d.storage));
    if (d.guest_tests) {
        const auto& report = *d.guest_tests;
        std::printf("MIPS %u/%u  hash %08lx\n", report.passed, report.count,
                    static_cast<unsigned long>(report.signature));
        for (unsigned i = 0; i < report.count; ++i)
            std::printf("%-4s %-24s\n", report.tests[i].passed ? "PASS" : "FAIL", report.tests[i].name);
        if (d.gt2_probe && d.gt2_probe->count)
            std::printf("GT2 hash %u/%u sig %08lx\n", d.gt2_probe->passed, d.gt2_probe->count,
                        static_cast<unsigned long>(d.gt2_probe->signature));
        else std::printf("GT2 probe: not built or gated     \n");
        if (d.interrupt_tests && d.boot_operations && d.bios_tests && d.dma_tests)
            std::printf("IRQ %u/%u Boot %u/%u BIOS %u/%u DMA %u/%u\n", d.interrupt_tests->passed,
                d.interrupt_tests->count, d.boot_operations->passed, d.boot_operations->count,
                d.bios_tests->passed, d.bios_tests->count, d.dma_tests->passed, d.dma_tests->count);
        if (d.gpu_tests) std::printf("GPU %u/%u ", d.gpu_tests->passed, d.gpu_tests->count);
        if (d.cdrom_tests) std::printf("CD %u/%u\n", d.cdrom_tests->passed, d.cdrom_tests->count);
        std::printf("                                     \r");
        for (unsigned i = 0; i < report.count; ++i) {
            if (!report.tests[i].passed) {
                std::printf("got %08lx expected %08lx",
                    static_cast<unsigned long>(report.tests[i].actual),
                    static_cast<unsigned long>(report.tests[i].expected));
                break;
            }
        }
    }
    gfxFlushBuffers();
    return true;
}

} // namespace opengt::ctr
