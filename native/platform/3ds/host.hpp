#pragma once

#include "opengt/platform.hpp"
#include <3ds.h>
#include <citro3d.h>

namespace opengt::ctr {

class Host final : public platform::Graphics, public platform::Input,
                   public platform::Timing {
public:
    Host() = default;
    ~Host() override;
    Host(const Host&) = delete;
    Host& operator=(const Host&) = delete;
    bool initialize() noexcept;
    bool running() noexcept;
    bool present(const platform::Diagnostics&) noexcept override;
    platform::InputState poll() noexcept override;
    std::uint64_t ticks() const noexcept override;
    std::uint64_t ticks_per_second() const noexcept override;
    void wait_vblank() noexcept override;
    std::size_t linear_free_bytes() const noexcept;
private:
    bool gfx_ready_{};
    bool c3d_ready_{};
    C3D_RenderTarget* top_{};
};

} // namespace opengt::ctr
