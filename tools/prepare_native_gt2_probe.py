#!/usr/bin/env python3
"""Validate the supported user disc; emit its exact 44-byte string-hash function.

Reuses OpenGTPS1's ISO reader and the function root documented by
prepare_reference.py. All extracted bytes and generated game code stay ignored.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

from psx_iso import PsxIso, directory_records
from generate_mips_tests import backend, ROOT

DISC_SIZE = 691850208
DISC_HASH = "d0ab6e70539601057590a36299543c0adad219254d712f7d4273219094ed5031"
EXE_HASH = "4dd40d01a3e83967e2d4301106890eb314d72027802bee077bbbc246f152e331"
ENTRY, LENGTH = 0x80083004, 44
VECTORS = [("empty", b""), ("single", b"A"), ("gt2", b"GT2"),
           ("title", b"Gran Turismo 2"), ("high_bytes", bytes([0x80,0xff,0x41])),
           ("wrap_128", b"A" * 128)]


def reference_hash(data):
    value = 0
    for byte in data:
        value = (((value << 6) | (value >> 26)) + byte) & 0xffffffff
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--image", type=Path, default=ROOT / "Gran Turismo 2 [Simulation Disc] [U] [SCUS-94488].img")
    args = parser.parse_args()
    image = args.image.resolve()
    if image.stat().st_size != DISC_SIZE: raise ValueError("unsupported disc size")
    cue = image.with_suffix(".cue")
    cue_text = cue.read_text(encoding="utf-8-sig")
    files = re.findall(r'^\s*FILE\s+"([^"]+)"\s+BINARY\s*$', cue_text, re.M | re.I)
    tracks = re.findall(r'^\s*TRACK\s+(\d+)\s+(\S+)\s*$', cue_text, re.M | re.I)
    if len(files) != 1 or (cue.parent / files[0]).resolve() != image or tracks != [("01", "MODE2/2352")]:
        raise ValueError("CUE must reference this image as one MODE2/2352 track")
    digest = hashlib.sha256()
    with image.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024*1024), b""): digest.update(chunk)
    if digest.hexdigest() != DISC_HASH: raise ValueError("unsupported disc SHA-256")
    with PsxIso(image) as iso:
        records = {name:(lba,size) for name,lba,size,flags in directory_records(iso.read_extent(*iso.root_record())) if not flags & 2}
        exe = iso.read_extent(*records["SCUS_944.88"])
    if hashlib.sha256(exe).hexdigest() != EXE_HASH or exe[:8] != b"PS-X EXE":
        raise ValueError("unexpected Simulation executable")
    base,size = struct.unpack_from("<II", exe, 0x18)
    offset = 0x800 + ENTRY - base
    if ENTRY < base or ENTRY + LENGTH > base + size or offset + LENGTH > len(exe):
        raise ValueError("hash function outside executable image")
    code = exe[offset:offset+LENGTH]
    spec = dict(name="gt2_string_hash", base=ENTRY, words=list(struct.unpack("<11I", code)))
    cpp = "// Locally extracted GT2 function: DO NOT COMMIT OR DISTRIBUTE.\n" + backend.emit([spec])
    cpp += '''
#include "opengt/guest_tests.hpp"
namespace opengt::guest {
TestReport run_gt2_probe() noexcept {
    TestReport report{};
    report.signature = 2166136261u;
'''
    evidence = []
    for name,data in VECTORS:
        expected = reference_hash(data)
        evidence.append(dict(name=name, length=len(data), expected=f"{expected:08X}"))
        cpp += f'''    {{
        Memory memory = reset_test_memory();
        const std::uint8_t input[] = {{{','.join(str(b) for b in data + bytes([0]))}}};
        bool ok = true;
        for (unsigned i = 0; i < sizeof(input); ++i) ok &= memory.write(0x80000400u + i, 1, input[i]);
        Context c; c.start({backend.literal(ENTRY)});
        c.write(4, 0x80000400u); c.write(31, 0x1FFF0000u);
        recompiled::gt2_string_hash(c, memory, 2000, 0x1FFF0000u);
        for (unsigned i = 0; i < sizeof(input); ++i) {{
            u32 value = 0; ok &= memory.read(0x80000400u + i, 1, value); ok &= value == input[i];
        }}
        ok &= c.stop == Stop::returned && c.read(2) == {backend.literal(expected)};
        ok &= c.read(4) == 0x80000400u + sizeof(input) && c.hi == 0 && c.lo == 0;
        report.tests[report.count++] = {{"{name}", ok, c.read(2), {backend.literal(expected)}}};
        report.passed += ok;
        report.signature = (report.signature ^ c.read(2)) * 16777619u;
    }}
'''
    cpp += "    return report;\n}\n}\n"
    output = ROOT / "generated/old3ds"
    output.mkdir(parents=True,exist_ok=True)
    work = ROOT / "work/native-gt2"
    work.mkdir(parents=True,exist_ok=True)
    (work / "SCUS_944.88").write_bytes(exe)
    (output / "gt2_probe.cpp").write_bytes(cpp.encode("utf-8"))
    (output / "gt2_hash_manifest.json").write_text(json.dumps({"functions":[spec]},indent=2)+"\n")
    provenance = dict(disc_sha256=DISC_HASH, executable_sha256=EXE_HASH, entry=f"0x{ENTRY:08X}",
                      bytes=LENGTH, function_sha256=hashlib.sha256(code).hexdigest(), vectors=evidence)
    (output / "gt2_probe_provenance.json").write_text(json.dumps(provenance,indent=2)+"\n")
    print(json.dumps(provenance,indent=2))
    print("Generated local GT2 C++ probe:", output / "gt2_probe.cpp")
    return image


if __name__ == "__main__":
    main()
