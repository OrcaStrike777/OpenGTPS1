#!/usr/bin/env python3
"""Regenerate-check and compile the portable guest suite in a compiler shell."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="cl" if sys.platform == "win32" else "c++")
    parser.add_argument("--gt2-probe", action="store_true")
    parser.add_argument("--boot", action="store_true")
    args = parser.parse_args()
    if args.boot: args.gt2_probe = True
    compiler = shutil.which(args.compiler)
    if not compiler: raise SystemExit("Compiler not on PATH. Use a Visual Studio developer prompt or --compiler.")
    subprocess.run([sys.executable, str(ROOT / "tools/generate_mips_tests.py"), "--check"], check=True)
    subprocess.run([sys.executable, str(ROOT / "tools/test_native_emitter.py")], check=True)
    subprocess.run([sys.executable, str(ROOT / "tools/generate_boot_tests.py"), "--check"], check=True)
    sources = [ROOT / "native/runtime/guest.cpp", ROOT / "native/runtime/ps1_interrupts.cpp", ROOT / "native/runtime/ps1_dma.cpp", ROOT / "native/runtime/ps1_gpu.cpp", ROOT / "native/runtime/startup_services.cpp", ROOT / "native/tests/recompiled/mips_suite.cpp",
               ROOT / "native/tests/guest_tests_main.cpp",
               ROOT / ("generated/old3ds/gt2_probe.cpp" if args.gt2_probe else "native/tests/gt2_probe_stub.cpp")]
    sources += [ROOT / ("generated/old3ds/gt2_boot.cpp" if args.boot else "native/tests/boot_probe_stub.cpp")]
    sources += [ROOT / "native/tests/recompiled/boot_runtime_suite.cpp", ROOT / "native/tests/interrupt_tests.cpp", ROOT / "native/tests/bios_tests.cpp", ROOT / "native/tests/dma_tests.cpp", ROOT / "native/tests/gpu_tests.cpp"]
    if not all(p.is_file() for p in sources): raise SystemExit("Run tools/prepare_native_gt2_boot.py for --boot, or tools/prepare_native_gt2_probe.py for --gt2-probe.")
    build = ROOT / "build" / ("native-guest-boot" if args.boot else "native-guest-gt2" if args.gt2_probe else "native-guest")
    build.mkdir(parents=True, exist_ok=True)
    executable = build / ("guest_tests.exe" if sys.platform == "win32" else "guest_tests")
    if Path(compiler).name.lower() in ("cl", "cl.exe"):
        options = ["/nologo", "/std:c++17", "/O2", "/EHsc", "/W4", "/WX",
                   "/I" + str(ROOT / "native/include"), "/Fe:" + str(executable)]
    else:
        options = ["-std=c++17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                   "-I", str(ROOT / "native/include"), "-o", str(executable)]
    subprocess.run([compiler, *options, *map(str, sources)], cwd=build, check=True)
    subprocess.run([str(executable)], cwd=build, check=True)


if __name__ == "__main__":
    main()
