#pragma once

#include <cstddef>
#include <cstdint>

namespace opengt::guest { struct TestReport; struct BootReport; }

// No SDK or managed types may cross this boundary. Implementations are owned
// by the host and outlive their users. Guest execution remains single-threaded.
namespace opengt::platform {

enum class Status { ok, unavailable, unsupported, invalid_argument, io_error };
const char* status_name(Status status) noexcept;

enum Button : std::uint32_t {
    a = 1u << 0, b = 1u << 1, x = 1u << 2, y = 1u << 3,
    up = 1u << 4, down = 1u << 5, left = 1u << 6, right = 1u << 7,
    l = 1u << 8, r = 1u << 9, start = 1u << 10, select = 1u << 11,
};

struct InputState {
    std::uint32_t held{}, pressed{}, released{};
    // Normalized signed axes [-1, 1], positive right/up; no game binding yet.
    float circle_x{}, circle_y{};
};

class Input {
public:
    virtual ~Input() = default;
    virtual InputState poll() noexcept = 0;
};

class Timing {
public:
    virtual ~Timing() = default;
    virtual std::uint64_t ticks() const noexcept = 0;
    virtual std::uint64_t ticks_per_second() const noexcept = 0;
    // Host presentation pacing only; never a replacement for guest VBlank.
    virtual void wait_vblank() noexcept = 0;
};

struct Diagnostics {
    InputState input{};
    std::uint64_t frame{};
    double frame_ms{};
    std::size_t linear_free_bytes{};
    Status storage{Status::unavailable};
    const guest::TestReport* guest_tests{};
    const guest::TestReport* gt2_probe{};
    const guest::TestReport* boot_operations{};
    const guest::BootReport* boot{};
    bool boot_page{true};
};

class Graphics {
public:
    virtual ~Graphics() = default;
    // Bootstrap clear/presentation only. PS1 command submission comes later.
    virtual bool present(const Diagnostics& diagnostics) noexcept = 0;
};

class Audio {
public:
    virtual ~Audio() = default;
    // Interleaved stereo signed 16-bit PCM. Caller retains buffer ownership.
    // Unsupported/unavailable must not be reported as successful playback.
    virtual Status submit(const std::int16_t* samples, std::size_t frames,
                          std::uint32_t sample_rate) noexcept = 0;
    virtual void stop() noexcept = 0;
};

class FileSystem {
public:
    virtual ~FileSystem() = default;
    // Root-relative paths. Short reads at EOF are successful; bytes_read tells
    // the caller what was returned. No implicit whole-file allocation.
    virtual Status read_at(const char* path, std::uint64_t offset, void* output,
                           std::size_t capacity, std::size_t& bytes_read) noexcept = 0;
};

class Saves {
public:
    virtual ~Saves() = default;
    // Whole raw PS1 card images, slots 0/1. A future implementation must verify
    // size and preserve the previous card on failed replacement. No guest
    // memory-card protocol is implemented by this storage interface.
    virtual Status load(unsigned slot, void* image, std::size_t size) noexcept = 0;
    virtual Status store(unsigned slot, const void* image, std::size_t size) noexcept = 0;
};

class SilentAudio final : public Audio {
public:
    Status submit(const std::int16_t*, std::size_t, std::uint32_t) noexcept override {
        return Status::unsupported;
    }
    void stop() noexcept override {}
};

class UnavailableSaves final : public Saves {
public:
    Status load(unsigned, void*, std::size_t) noexcept override { return Status::unsupported; }
    Status store(unsigned, const void*, std::size_t) noexcept override { return Status::unsupported; }
};

} // namespace opengt::platform
