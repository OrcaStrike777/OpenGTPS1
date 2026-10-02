#include "opengt/stdio_files.hpp"

#include <climits>
#include <cstdio>
#include <cstring>
#include <limits>
#include <initializer_list>

using namespace opengt::platform;

namespace {
int failures = 0;
void check(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
}

// Run in a disposable build directory. No game files are read or written.
int main() {
    const char* fixture = "platform-io-test.txt";
    std::FILE* file = std::fopen(fixture, "wb");
    if (!file) return 2;
    const bool wrote = std::fwrite("0123456789", 1, 10, file) == 10;
    const bool closed = std::fclose(file) == 0;
    if (!wrote || !closed) return 2;

    StdioFiles files(".");
    char output[8]{};
    std::size_t count = 99;
    check(files.read_at(fixture, 3, output, 4, count) == Status::ok &&
          count == 4 && std::memcmp(output, "3456", 4) == 0, "bounded offset read");
    check(files.read_at(fixture, 8, output, sizeof(output), count) == Status::ok &&
          count == 2 && std::memcmp(output, "89", 2) == 0, "short EOF read");
    check(files.read_at(fixture, 100, output, sizeof(output), count) == Status::ok &&
          count == 0, "read beyond EOF");
    check(files.read_at(fixture, 0, nullptr, 0, count) == Status::ok && count == 0,
          "zero capacity needs no buffer");
    check(files.read_at(fixture, 0, nullptr, 1, count) == Status::invalid_argument && count == 0,
          "null output rejected");
    for (const char* path : {"", "../carda.sav", "a/../b", "./a", "a//b", "a/",
                             "/absolute", "C:/absolute", "a\\b"}) {
        count = 99;
        check(files.read_at(path, 0, output, sizeof(output), count) == Status::invalid_argument &&
              count == 0, "invalid relative path rejected");
    }
    check(files.read_at("platform-io-missing/file", 0, output, sizeof(output), count) ==
          Status::unavailable && count == 0, "missing file is distinguishable");
    check(files.read_at(fixture, std::numeric_limits<std::uint64_t>::max(), output,
                        sizeof(output), count) == Status::unsupported && count == 0,
          "unrepresentable offset rejected");
    char long_path[600];
    std::memset(long_path, 'a', sizeof(long_path) - 1);
    long_path[sizeof(long_path) - 1] = '\0';
    check(files.read_at(long_path, 0, output, sizeof(output), count) == Status::invalid_argument,
          "path overflow rejected");
    StdioFiles invalid(nullptr);
    check(invalid.read_at(fixture, 0, output, sizeof(output), count) == Status::invalid_argument,
          "null root rejected");
    SilentAudio audio;
    UnavailableSaves saves;
    check(audio.submit(nullptr, 0, 44100) == Status::unsupported, "silent backend is explicit");
    check(saves.store(0, output, sizeof(output)) == Status::unsupported,
          "unimplemented save must never claim success");
    std::remove(fixture);
    if (!failures) std::puts("Platform I/O tests passed");
    return failures ? 1 : 0;
}
