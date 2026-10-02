#pragma once

#include "opengt/platform.hpp"

namespace opengt::platform {

// newlib/libctru and desktop CRT implementation. Root must outlive this object.
// Read-only bootstrap adapter; no directory creation or save writes.
class StdioFiles final : public FileSystem {
public:
    explicit StdioFiles(const char* root) noexcept : root_(root) {}
    Status read_at(const char* path, std::uint64_t offset, void* output,
                   std::size_t capacity, std::size_t& bytes_read) noexcept override;
private:
    const char* root_;
};

} // namespace opengt::platform
