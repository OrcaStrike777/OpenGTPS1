#pragma once
#include "opengt/guest.hpp"

namespace opengt::guest {
struct TestResult { const char* name{}; bool passed{}; u32 actual{}, expected{}; };
struct TestReport { unsigned count{}, passed{}; u32 signature{}; TestResult tests[24]{}; };
// Same generated suite on host and ARM11. Static 2 MiB RAM + 1 KiB scratch;
// no allocations, game data, files, graphics, or audio. Not reentrant.
TestReport run_synthetic_tests() noexcept;
// Optional, locally generated GT2 probe; default build reports zero cases.
TestReport run_gt2_probe() noexcept;
// Shared fixture arena avoids allocating a second 2 MiB guest RAM for probes.
Memory reset_test_memory() noexcept;
} // namespace opengt::guest
