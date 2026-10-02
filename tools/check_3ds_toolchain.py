#!/usr/bin/env python3
"""Read-only bootstrap dependency check; requires no game data or packages."""
import os
from pathlib import Path
import shutil
import sys


def main():
    missing = []
    roots = {}
    for name in ("DEVKITPRO", "DEVKITARM"):
        value = os.environ.get(name)
        if not value:
            missing.append(f"{name} is unset")
        else:
            roots[name] = Path(value)
            if not roots[name].is_dir():
                missing.append(f"{name} directory does not exist: {value}")
    suffix = ".exe" if os.name == "nt" else ""
    for program, root, relative in (
        ("arm-none-eabi-g++", "DEVKITARM", "bin"),
        ("3dsxtool", "DEVKITPRO", "tools/bin"),
        ("smdhtool", "DEVKITPRO", "tools/bin"),
    ):
        candidate = roots.get(root, Path("/__missing_devkit_root__")) / relative / (program + suffix)
        if not candidate.is_file() and not shutil.which(program):
            missing.append(f"{program} missing (expected under ${root}/{relative})")
    if not shutil.which("make"):
        missing.append("GNU make missing from PATH; run in the devkitPro shell")
    for root, relative in (
        ("DEVKITARM", "3ds_rules"),
        ("DEVKITPRO", "libctru/include/3ds.h"),
        ("DEVKITPRO", "libctru/include/citro3d.h"),
        ("DEVKITPRO", "libctru/lib/libctru.a"),
        ("DEVKITPRO", "libctru/lib/libcitro3d.a"),
        ("DEVKITPRO", "libctru/default_icon.png"),
    ):
        if root not in roots or not (roots[root] / relative).is_file():
            missing.append(f"${root}/{relative} missing")
    if missing:
        print("3DS build dependencies unavailable:")
        for item in missing:
            print(f"  - {item}")
        print("Install devkitPro's 3ds-dev package group; see platform/3ds/README.md.")
        return 1
    print("3DS bootstrap dependency paths found. This is not a compiler or hardware test.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
