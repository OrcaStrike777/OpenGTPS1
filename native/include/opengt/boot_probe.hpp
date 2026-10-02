#pragma once
#include "opengt/guest.hpp"
#include "opengt/guest_tests.hpp"
namespace opengt::guest {
struct BootReport {
    bool available{}, passed{}, clears_verified{};
    u32 entry{}, pc{}, last_pc{}, last_function{}, functions{}, instructions{};
    u32 unresolved{}, ra{}, sp{}, io_accesses{};
    Stop stop{Stop::running};
    u32 trace[16]{}; unsigned trace_count{};
};
// Stops synchronously within a bounded instruction budget; no host recursion.
BootReport run_boot_probe(bool mask_shim = true, u32 budget = 2000000) noexcept;
TestReport run_boot_runtime_tests() noexcept;
}
