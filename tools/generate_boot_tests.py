#!/usr/bin/env python3
"""Supplemental native startup tests; preserves the established MIPS 16 hash."""
import argparse
from generate_mips_tests import ROOT, backend, i, r

def source():
    specs = [dict(name=f"store_{op}_{offset}",base=0x80010000,
                  words=[i(op,8,4,offset),r(8,rs=31),0])
             for op in (42,46) for offset in range(4)]
    specs += [dict(name='syscall_resume',base=0x80010000,
                   words=[i(9,2,0,7),i(9,4,0,2),0x0000000C,r(8,rs=31),0]),
              dict(name='syscall_load',base=0x80010000,
                   words=[i(35,4,8,0),0x03FFFFCC,r(8,rs=31),0]),
              dict(name='syscall_slot',base=0x80010000,
                   words=[i(4,0,0,2),0x0000000C,0,0])]
    text = '// Generated synthetic store-merge fixtures. No game data.\n' + backend.emit(specs)
    text += '''
#include "opengt/boot_probe.hpp"
#include "opengt/startup_services.hpp"
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
    {
        StartupBios bios; Context c; c.start(0x80010000); c.write(31,0x1FFF0000);
        recompiled::syscall_resume(c,m,10,0x1FFF0000);
        ok = c.stop==Stop::syscall && c.instructions==3 && c.pc==0x80010008 && c.epc==c.pc;
        ok &= bios.dispatch_syscall(c) && c.pc==0x8001000C && c.read(2)==7 && c.sr==0x401;
        recompiled::syscall_resume(c,m,10,0x1FFF0000);
        ok &= c.stop==Stop::returned && c.instructions==5 && bios.syscall_calls==1;
        report.tests[report.count++] = {"SYS_emitted_resume",ok,ok,1}; report.passed += ok;
    }
    {
        StartupBios bios; Context c; c.start(0x80010000); c.write(31,0x1FFF0000);
        c.write(4,99); c.write(8,0x100); c.sr=0x401; m.write(0x100,4,1);
        recompiled::syscall_load(c,m,10,0x1FFF0000);
        // Exception observes the retired load; the instruction's code field
        // does not select a BIOS API. No pending load survives exception entry.
        ok = c.stop==Stop::syscall && c.pending_register==0 && c.read(4)==1 && c.instructions==2;
        ok &= bios.dispatch_syscall(c) && c.read(2)==1 && (c.sr&0x401)==0;
        recompiled::syscall_load(c,m,10,0x1FFF0000);
        ok &= c.stop==Stop::returned && c.instructions==4;
        report.tests[report.count++] = {"SYS_code_load_retirement",ok,ok,1}; report.passed += ok;
    }
    {
        StartupBios bios; Context c; c.start(0x80010000); c.write(4,2);
        c.cause=0x500; c.bad_vaddr=0xABCDEF00;
        recompiled::syscall_slot(c,m,10,0x1FFF0000);
        ok = c.stop==Stop::syscall && c.pc==0x80010004 && c.epc==0x80010000 &&
             c.cause==0x80000520 && c.bad_vaddr==0xABCDEF00 && c.instructions==2 &&
             !bios.dispatch_syscall(c) && bios.syscall_calls==0;
        report.tests[report.count++] = {"SYS_delay_slot_diagnostic",ok,ok,1}; report.passed += ok;
    }
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
