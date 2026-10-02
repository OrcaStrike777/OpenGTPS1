#include "opengt/stdio_files.hpp"

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstring>

namespace opengt::platform {

const char* status_name(Status status) noexcept {
    switch (status) {
    case Status::ok: return "ok";
    case Status::unavailable: return "unavailable";
    case Status::unsupported: return "unsupported";
    case Status::invalid_argument: return "invalid argument";
    case Status::io_error: return "I/O error";
    }
    return "unknown";
}

namespace {
bool relative_path(const char* path) noexcept {
    if (!path || !*path || *path == '/') return false;
    const char* component = path;
    for (const char* p = path;; ++p) {
        if (*p == ':' || *p == '\\') return false;
        if (*p == '/' || *p == '\0') {
            const auto length = p - component;
            if (length == 0 || (length == 1 && component[0] == '.') ||
                (length == 2 && component[0] == '.' && component[1] == '.')) return false;
            if (*p == '\0') return true;
            component = p + 1;
        }
    }
}
}

Status StdioFiles::read_at(const char* path, std::uint64_t offset, void* output,
                          std::size_t capacity, std::size_t& bytes_read) noexcept {
    bytes_read = 0;
    if (!root_ || !*root_ || !relative_path(path) || (!output && capacity != 0))
        return Status::invalid_argument;
    // Explicitly reject offsets this CRT cannot seek; never silently truncate.
    if (offset > static_cast<std::uint64_t>(LONG_MAX)) return Status::unsupported;
    char full_path[512];
    const int count = std::snprintf(full_path, sizeof(full_path), "%s/%s", root_, path);
    if (count < 0 || static_cast<std::size_t>(count) >= sizeof(full_path))
        return Status::invalid_argument;
    std::FILE* file = std::fopen(full_path, "rb");
    if (!file) return errno == ENOENT ? Status::unavailable : Status::io_error;
    Status result = Status::ok;
    if (std::fseek(file, static_cast<long>(offset), SEEK_SET) != 0) {
        result = Status::io_error;
    } else if (capacity != 0) {
        bytes_read = std::fread(output, 1, capacity, file);
        if (std::ferror(file)) result = Status::io_error;
    }
    if (std::fclose(file) != 0) result = Status::io_error;
    return result;
}

} // namespace opengt::platform
