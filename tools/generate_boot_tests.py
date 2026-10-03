#!/usr/bin/env python3
"""Supplemental native startup tests; preserves the established MIPS 16 hash."""
import argparse
from generate_mips_tests import ROOT, backend, i, r

def source():
    specs = [dict(name=f"store_{op}_{offset}",base=0x80010000,
                  words=[i(op,8,4,offset),r(8,rs=31),0])
             for op in (42,46) for offset in range(4)]
    text = '// Generated synthetic store-merge fixtures. No game data.\n' + backend.emit(specs)
    text += '''
#include "opengt/boot_probe.hpp"
namespace opengt::guest {
TestReport run_boot_runtime_tests() noexcept {
    TestReport report{};
    Memory m = reset_test_memory();
    bool ok = true;
'''
    expected = {42:[0xDDCCBB11,0xDDCC1122,0xDD112233,0x11223344],
                46:[0x11223344,0x223344AA,0x3344BBAA,0x44CCBBAA]}
    for op in (42,46):
        for offset in range(4):
            text += f'''    {{
        m.write(0x100,4,0xDDCCBBAA); Context c; c.start(0x80010000);
        c.write(4,0x80000100); c.write(8,0x11223344); c.write(31,0x1FFF0000);
        recompiled::store_{op}_{offset}(c,m,10,0x1FFF0000);
        u32 v=0; ok &= m.read(0x100,4,v) && v=={backend.literal(expected[op][offset])} && c.stop==Stop::returned;
    }}
'''
    text += '''    report.tests[report.count++] = {"SWL_SWR_all_offsets",ok,ok,1}; report.passed += ok;
    InterruptController io; u32 value=0;
    ok = !m.read(0x1F801074,2,value);
    m.attach_interrupts(&io);
    ok &= m.read(0x1F801074,2,value) && value==0;
    ok &= m.write(0x9F801074,2,0xFFFF);
    ok &= m.read(0xBF801074,4,value) && value==0x7FF;
    ok &= m.write(0x1F801070,2,0) && !m.read(0x1F801075,2,value);
    ok &= !m.read(0xDF801074,2,value) && !m.read(0x1F801074,1,value);
    ok &= io.mask_reads==2 && io.mask_writes==1 && io.stat_writes==1;
    m.attach_interrupts(nullptr);
    ok &= !m.read(0x1F801074,2,value);
    report.tests[report.count++] = {"I_MASK_opt_in",ok,ok,1}; report.passed += ok;
    return report;
}
}
'''
    return text

def main():
    parser=argparse.ArgumentParser(); parser.add_argument('--check',action='store_true'); args=parser.parse_args()
    path=ROOT/'native/tests/recompiled/boot_runtime_suite.cpp'; text=source()
    if args.check:
        if path.read_text()!=text: raise SystemExit('Stale boot runtime fixtures')
    else: path.write_bytes(text.encode())
    print('Boot runtime fixtures checked' if args.check else 'Boot runtime fixtures generated')

if __name__=='__main__': main()
