#!/usr/bin/env python3
"""Build a bounded sparse startup graph from the validated original EXE.

No instructions are replaced. Explicit audited ranges end at the observed
interrupt-setup boundary; additional calls stop as unknown targets.
"""
import hashlib
import json
import re
import struct
from prepare_native_gt2_probe import main as validate_disc, EXE_HASH
from generate_mips_tests import ROOT, backend

RANGES = [(0x8005D600,0x8005D698), (0x8008CE08,0x8008CE30),
          (0x8008DDB4,0x8008DE24), (0x8008DD74,0x8008DDB4),
          (0x80010998,0x800109B0), (0x8005D9BC,0x8005D9F0),
          (0x8008CE30,0x8008CEDC), (0x8008BC78,0x8008BCA8),
          (0x8008BE0C,0x8008BE64)]

def generate(exe, system_cnf):
    if hashlib.sha256(exe).hexdigest() != EXE_HASH: raise ValueError('wrong executable')
    entry,gp,base,size = struct.unpack_from('<4I',exe,0x10)
    sp,spoff = struct.unpack_from('<2I',exe,0x30)
    if not sp:
        match = re.search(r'STACK\s*=\s*([0-9a-fA-F]+)',system_cnf)
        if not match: raise ValueError('missing SYSTEM.CNF stack')
        sp = int(match[1],16)
    sp = (sp+spoff)&0xffffffff
    if entry != RANGES[0][0] or base != 0x80010000 or size != 0x99000 or len(exe)!=size+0x800:
        raise ValueError('unsupported layout')
    image = exe[0x800:]
    # PS-X EXE .data fields are not treated as separate BSS. Original CRT clears it.
    out = ['// Locally generated proprietary startup code; keep ignored.',
           '#include "opengt/guest_tests.hpp"', '#include "opengt/boot_probe.hpp"',
           'namespace opengt::guest {', 'namespace {',
           'const std::uint8_t image[] = {']
    out += [','.join(str(x) for x in image[i:i+64])+',' for i in range(0,len(image),64)]
    out += ['};', '}', 'BootReport run_boot_probe(bool mask_shim, u32 budget) noexcept {',
            'BootReport report{}; report.available = true;',
            'Memory m = reset_test_memory(); BootIo io{};',
            'if (mask_shim) m.attach_boot_io(&io);',
            f'for (u32 i=0;i<sizeof(image);++i) if (!m.write({backend.literal(base)}+i,1,image[i])) return report;',
            '// Poison zero-initialized BSS to prove the original guest loops clear it.',
            'for (u32 a=0x800A8D5C;a<0x801F0D60;a+=4) m.write(a,4,0xA5A5A5A5);',
            f'Context c; c.start({backend.literal(entry)}); c.write(28,{backend.literal(gp)}); c.write(29,{backend.literal(sp)});',
            f'report.entry={backend.literal(entry)};',
            'while (c.stop == Stop::running) {',
            'if (!budget--) { c.stop=Stop::budget; break; }',
            'if(c.pc&3){c.in_delay=c.next_delay;c.fault(Stop::address_load,c.pc);break;}',
            'switch(c.pc) {']
    for start,end in RANGES:
        for pc in range(start,end,4):
            word=struct.unpack_from('<I',exe,0x800+pc-base)[0]
            statement,control=backend.operation(word,pc)
            out += [f'case {backend.literal(pc)}: {{',
                    f'const u32 s=c.read({(word>>21)&31}),t=c.read({(word>>16)&31});',
                    '(void)s;(void)t;const u32 next=c.begin();']
            if pc==start:
                out += ['++report.functions;report.last_function=c.pc;',
                        'if(report.trace_count<16) report.trace[report.trace_count++]=c.pc;']
            if control: out += ['if(c.in_delay){ c.stop=Stop::delay_control; break; }']
            out += [statement, 'if(c.stop==Stop::running){report.last_pc=c.pc;c.pc=next;} break; }']
    out += ['default:c.stop=Stop::unknown_pc;break;', '}', '}',
            'report.pc=c.pc;report.instructions=c.instructions;report.stop=c.stop;',
            'report.unresolved=c.stop==Stop::unmapped?c.bad_vaddr:c.pc;',
            'report.ra=c.read(31);report.sp=c.read(29);report.io_accesses=io.accesses;',
            'report.clears_verified=true;u32 value=0;',
            'for(u32 a=0x800A8D5C;a<0x801F0D60;a+=4) if(!m.read(a,4,value)||value) report.clears_verified=false;',
            'const bool heap=m.read(0x800A8D50,4,value)&&value==0x801F0D60;',
            'report.passed=report.clears_verified&&heap&&report.functions==9&&c.stop==Stop::unmapped&&',
            '(mask_shim?(c.pc==0x8008BE50&&c.bad_vaddr==0x1F801070&&io.accesses==2):',
            '(c.pc==0x8008BE44&&c.bad_vaddr==0x1F801074&&io.accesses==0));',
            'return report;', '}', '}']
    return '\n'.join(out)+'\n',dict(entry=hex(entry),gp=hex(gp),sp=hex(sp),load=hex(base),size=size,
                                   ranges=[(hex(a),hex(b)) for a,b in RANGES],exe_sha256=EXE_HASH)

def main():
    path = validate_disc()
    # Reuse the supported ISO reader for SYSTEM.CNF, not a made-up startup stack.
    from psx_iso import PsxIso,directory_records
    with PsxIso(path) as iso:
        records={n:(l,s) for n,l,s,f in directory_records(iso.read_extent(*iso.root_record()))}
        cnf=iso.read_extent(*records['SYSTEM.CNF']).decode('ascii')
    code,meta=generate((ROOT/'work/native-gt2/SCUS_944.88').read_bytes(),cnf)
    dest=ROOT/'generated/old3ds'
    (dest/'gt2_boot.cpp').write_bytes(code.encode())
    (dest/'gt2_boot_layout.json').write_text(json.dumps(meta,indent=2)+'\n')
    print(json.dumps(meta,indent=2))

if __name__=='__main__': main()
