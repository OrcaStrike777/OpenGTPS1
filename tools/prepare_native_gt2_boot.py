#!/usr/bin/env python3
"""Build a bounded sparse startup graph from the validated original EXE.

No instructions are replaced. Explicit audited ranges reach the original
VBlank callbacks and CdInit's bank-1 CD-ROM interrupt-flags read; other calls stop as unknown targets.
"""
import hashlib
import json
import re
import struct
from prepare_native_gt2_probe import main as validate_disc, EXE_HASH
from generate_mips_tests import ROOT, backend

RANGES = [(0x8005D600,0x8005D698), (0x8008CE08,0x8008CE30),
          (0x8008DDB4,0x8008DE24), (0x8008DD74,0x8008DDB4),
          (0x80010998,0x800109C0), (0x8005D9BC,0x8005D9F0),
          (0x8008CE30,0x8008CEDC), (0x8008BC78,0x8008BCA8),
          (0x8008BE0C,0x8008BEE4), (0x8008C314,0x8008C338),
          (0x8007AD58,0x8007AD90), (0x8008CC78,0x8008CC84), (0x8008C548,0x8008C5A0),
          (0x8008C638,0x8008C65C), (0x8008BCA8,0x8008BCD8),
          (0x8008C0B4,0x8008C1FC), (0x8008C998,0x8008C9A4),
          (0x8008CC58,0x8008CC64), (0x8008C668,0x8008C6B4),
          (0x8008C8E0,0x8008C904), (0x8008CC90,0x8008CC9C),
          (0x8008C948,0x8008C958), (0x80010954,0x80010998),
          (0x8008BD08,0x8008BD3C), (0x8008C60C,0x8008C638),
          (0x8008BEE4,0x8008C0B4), (0x8008C5A0,0x8008C60C),
          (0x80010928,0x80010954), (0x8007F830,0x8007F848),
          (0x8008CCA8,0x8008CCB4),
          (0x80089F38,0x80089F50), (0x80089FC8,0x80089FD8),
          (0x8008B6D8,0x8008B768), (0x8008E00C,0x8008E018),
          (0x8008DFF4,0x8008E000)]

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
           '#include "opengt/guest_tests.hpp"', '#include "opengt/ps1_dma.hpp"', '#include "opengt/ps1_gpu.hpp"', '#include "opengt/ps1_cdrom.hpp"', '#include "opengt/startup_services.hpp"', '#include "opengt/boot_probe.hpp"',
           'namespace opengt::guest {', 'namespace {',
           'const std::uint8_t image[] = {']
    out += [','.join(str(x) for x in image[i:i+64])+',' for i in range(0,len(image),64)]
    out += ['};', '}', 'BootReport run_boot_probe(bool devices, u32 budget, bool schedule_vblank) noexcept {',
            'BootReport report{}; report.available = true;',
            'Memory m = reset_test_memory(); InterruptController io{}; DmaInterrupts dicr(io); DmaPriority dma{}; TimerSetup timers{}; StartupBios bios{};',
            'VBlankClock video{}; GpuControl gpu(video); CdromRegisters cd;',
            'if (devices) { m.attach_interrupts(&io); m.attach_dma_priority(&dma); m.attach_dma_interrupts(&dicr); m.attach_timer_setup(&timers); m.attach_gpu_control(&gpu); m.attach_cdrom(&cd); }',
            f'for (u32 i=0;i<sizeof(image);++i) if (!m.write({backend.literal(base)}+i,1,image[i])) return report;',
            '// Poison zero-initialized BSS to prove the original guest loops clear it.',
            'for (u32 a=0x800A8D5C;a<0x801F0D60;a+=4) m.write(a,4,0xA5A5A5A5);',
            f'Context c; c.start({backend.literal(entry)}); c.write(28,{backend.literal(gp)}); c.write(29,{backend.literal(sp)});',
            f'report.entry={backend.literal(entry)};',
            'while (c.stop == Stop::running || c.stop == Stop::syscall) {',
            'io.sync_cpu(c);',
            'if (!budget--) { c.stop=Stop::budget; break; }',
            '// Native BIOS transitions consume budget too; no zero-cost dispatch loop.',
            'if (c.stop == Stop::syscall) { const u32 trap_pc=c.pc; if (devices && bios.dispatch_syscall(c)) { report.last_pc=trap_pc; continue; } break; }',
            'if (devices && (bios.dispatch(c) || bios.dispatch_console(c,m))) continue;',
            'if (devices && schedule_vblank && bios.dispatch_interrupt(c,m,io)) continue;',
            'if (c.stop != Stop::running) break;',
            'if(c.pc&3){c.in_delay=c.next_delay;c.fault(Stop::address_load,c.pc);break;}',
            'switch(c.pc) {']
    for start,end in RANGES:
        for pc in range(start,end,4):
            word=struct.unpack_from('<I',exe,0x800+pc-base)[0]
            statement,control=backend.operation(word,pc)
            out += [f'case {backend.literal(pc)}: {{',
                    f'const u32 s=c.read({(word>>21)&31}),t=c.read({(word>>16)&31});',
                    '(void)s;(void)t;const u32 next=c.begin();',
                    '// Provisional instruction timing: one CPU cycle per guest instruction.',
                    'if(devices && schedule_vblank) video.advance(1,io);']
            if pc==start:
                out += ['++report.functions;report.last_function=c.pc;',
                        'if(report.trace_count<64) report.trace[report.trace_count++]=c.pc;']
            if control: out += ['if(c.in_delay){ c.stop=Stop::delay_control; break; }']
            out += [statement]
            if pc == 0x8008BE50:
                out += ['if(c.stop==Stop::running) report.crossed_istat=true;']
            if pc == 0x8008C698:
                out += ['if(c.stop==Stop::running) report.crossed_dicr=true;']
            if pc == 0x8001096C:
                out += ['if(c.stop==Stop::running) ++report.vblank_polls;']
            if pc == 0x80010928:
                out += ['report.entered_vblank_callback=true;']
            if pc == 0x8007F844:
                out += ['report.gpu_control_value=t;']
            out += ['if(c.stop==Stop::running){report.last_pc=c.pc;c.pc=next;} break; }']
    out += ['default:c.stop=(c.pc==0xA0||c.pc==0xB0||c.pc==0xC0)?Stop::bios:Stop::unknown_pc;break;', '}', '}',
            'report.puts_calls=bios.puts_calls;report.printf_calls=bios.printf_calls;report.console_size=bios.console_size;',
            'static_assert(sizeof(report.console)==sizeof(bios.console));',
            'for(unsigned i=0;i<sizeof(report.console);++i) report.console[i]=bios.console[i];',
            'report.cd_writes=cd.writes;report.cd_reads=cd.reads;report.cd_bank=cd.bank();report.cd_status=cd.status();',
            'report.gpu_writes=gpu.writes;report.gpu_reads=gpu.reads;report.gpu_status=gpu.status();',
            'report.pc=c.pc;report.instructions=c.instructions;report.stop=c.stop;',
            'report.unresolved=c.stop==Stop::unmapped?c.bad_vaddr:c.pc;',
            'report.ra=c.read(31);report.sp=c.read(29);report.stat_reads=io.stat_reads;report.stat_writes=io.stat_writes;report.mask_reads=io.mask_reads;report.mask_writes=io.mask_writes;report.io_accesses=io.stat_reads+io.stat_writes+io.mask_reads+io.mask_writes;report.dma_reads=dma.reads;report.dma_writes=dma.writes;',
            'report.timer_writes=timers.writes;report.bios_api=c.stop==Stop::bios?c.read(9):0;report.bios_calls=bios.calls;report.hook_buffer=bios.hook_buffer;report.irq_pending=io.pending();report.irq_mask=io.mask();',
            'report.pad_auto_ack=bios.pad_auto_ack;report.vblank_auto_ack=bios.timer_auto_ack[3];report.pad_calls=bios.pad_calls;report.rcnt_calls=bios.rcnt_calls;',
            'report.sr=c.sr;report.cause=c.cause;report.epc=c.epc;report.syscall_api=c.stop==Stop::syscall?c.read(4):0;',
            'report.syscall_calls=bios.syscall_calls;report.critical_entries=bios.critical_entries;report.critical_exits=bios.critical_exits;',
            'report.cd_remove_calls=bios.cd_remove_calls;report.cd_events_open=bios.cd_events_open;report.cd_close_attempts=bios.cd_close_attempts;report.cd_dequeue_attempts=bios.cd_dequeue_attempts;report.cd_dequeue_unresolved=bios.cd_dequeue_unresolved;',
            'report.vblank_edges=video.edges;report.vblank_phase=video.phase;report.irq_entries=bios.irq_entries;report.irq_returns=bios.irq_returns;report.irq_deferred=bios.irq_deferred;report.irq_active=bios.irq_active;report.irq_resume_pc=bios.irq_resume_pc;report.irq_hook_pc=bios.irq_hook_pc;',
            'report.dicr_reads=dicr.reads;report.dicr_writes=dicr.writes;report.dicr_state=dicr.state();report.dma_completions=dicr.completions;report.dma_irq_rises=dicr.irq_rises;report.dma_channel_reads=dicr.channel_reads;report.dma_channel_writes=dicr.channel_writes;',
            'report.clears_verified=true;u32 value=0;',
            'const bool vblank=m.read(0x800A8C54,4,report.vblank_callback)&&m.read(0x80011DF4,4,report.vblank_counter);',
            'const bool handler=m.read(0x800A8C64,4,report.sdk_vblank_counter)&&m.read(0x800A7B7E,2,report.guest_in_interrupt);',
            'report.waiting_vblank=vblank&&c.stop==Stop::budget&&c.pc>=0x8001096C&&c.pc<=0x8001097C&&report.vblank_callback==0x80010928&&report.vblank_counter<4;',
            'for(u32 a=0x800A8D5C;a<0x801F0D60;a+=4) if(!m.read(a,4,value)||value) report.clears_verified=false;',
            'const bool heap=m.read(0x800A8D50,4,value)&&value==0x801F0D60;',
            'const char expected_console[]="CD_init:addr=800a7aec\\r\\n";',
            'bool console_ok=bios.console_size==sizeof(expected_console)-1;for(unsigned i=0;i<sizeof(expected_console);++i) console_ok=console_ok&&bios.console[i]==expected_console[i];',
            'report.passed=report.clears_verified&&heap&&',
            '(devices?(report.crossed_istat&&report.crossed_dicr&&vblank&&handler&&',
            'dicr.reads==0&&dicr.writes==1&&dicr.state()==0&&dicr.completions==0&&dicr.irq_rises==0&&',
            'dicr.channel_reads==0&&dicr.channel_writes==0&&io.pending()==0&&',
            'io.mask_writes==(schedule_vblank?7u:5u)&&io.mask()==(schedule_vblank?13u:9u)&&',
            'dma.writes==1&&dma.value==0x33333333&&timers.writes==1&&timers.mode[1]==0x100&&',
            'bios.calls==(schedule_vblank?10u:4u)&&bios.hook_buffer==0x800A7BB4&&bios.pad_calls==1&&bios.pad_auto_ack==0&&',
            'bios.cd_remove_calls==1&&bios.cd_events_open==0&&bios.cd_close_attempts==5&&bios.cd_dequeue_attempts==1&&bios.cd_dequeue_unresolved&&',
            'bios.syscall_calls==1&&bios.critical_entries==1&&bios.critical_exits==1&&',
            'bios.rcnt_calls==1&&bios.timer_auto_ack[3]==0&&',
            '(schedule_vblank?(',
            'c.stop==Stop::unmapped&&c.pc==0x8008B764&&c.bad_vaddr==0x1F801803&&c.read(2)==0x1F801803&&',
            'report.functions==58&&report.instructions==3397016&&report.last_pc==0x8008B760&&',
            'video.edges==6&&video.phase==742&&bios.irq_entries==4&&bios.irq_returns==4&&!bios.irq_active&&',
            'bios.irq_resume_pc==0x80010974&&bios.irq_hook_pc==0x8008BE74&&',
            'report.entered_vblank_callback&&report.sdk_vblank_counter==4&&report.guest_in_interrupt==0&&',
            'report.vblank_counter==4&&report.vblank_callback==0&&gpu.writes==4&&gpu.reads==0&&gpu.status()==0x14802000&&',
            'report.vblank_polls==417691&&io.stat_reads==12&&io.stat_writes==5&&io.mask_reads==16&&',
            'cd.writes==1&&cd.reads==0&&cd.bank()==1&&cd.status()==0x19&&',
            'bios.puts_calls==1&&bios.printf_calls==1&&console_ok&&',
            'm.read(0x800A7B88,4,value)&&value==0x8008BA2C&&',
            'c.sr==0x401&&c.cause==0x1C&&c.epc==0x8008B764):(',
            'report.vblank_counter==0&&report.vblank_callback==0x80010928&&gpu.writes==0&&',
            'report.waiting_vblank&&c.pc==0x80010974&&report.functions==27&&report.instructions==1999995&&report.last_pc==0x80010970&&',
            'report.vblank_polls==138471&&io.stat_reads==0&&io.stat_writes==1&&io.mask_reads==3&&',
            'video.edges==0&&bios.irq_entries==0&&!bios.irq_active&&!report.entered_vblank_callback&&',
            'c.sr==0x401&&c.cause==0x20&&c.epc==0x8008C94C))):',
            '(c.stop==Stop::unmapped&&c.pc==0x8008BE44&&c.bad_vaddr==0x1F801074&&report.io_accesses==0));',
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
