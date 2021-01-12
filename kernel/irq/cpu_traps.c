#include "cpu_traps.h"


__isr(fault_de)     { kpanic("Fault Division By Zero"); }
__isr(fault_db)     { kpanic("Fault Debug"); }
__isr(int_nomask)   { kpanic("Interrupt Nonmaskable"); }
__isr(fault_bp)     { kpanic("Breakpoint"); }
__isr(fault_of)     { kpanic("Fault Overflow"); }
__isr(fault_br)     { kpanic("Bound range exceeded"); }
__isr(fault_ud)     { kpanic("Invalid opcode"); }
__isr(fault_nm)     { kpanic("Device Not Available"); }
__isr(abort_df)     { kpanic("Double Fault"); }
__isr(fault_ts)     { kpanic("Invalid TSS"); }
__isr(fault_np)     { kpanic("Segment Not Present"); }
__isr(fault_ss)     { kpanic("Stack Segment Fault"); }
__isr(fault_gp)     { kpanic("General Protection Fault"); } 
__isr(fault_pf)     { kpanic("Page Fault"); }
__isr(fault_mf)     { kpanic("x87 Floating-Point Exception"); }
__isr(fault_ac)     { kpanic("Alignment Check"); }
__isr(abort_mc)     { kpanic("Machine Check"); }
__isr(fault_xmxf)   { kpanic("SIMD Floating Point Exception"); }
__isr(fault_ve)     { kpanic("Virtualization Exception"); }
__isr(fault_sx)     { kpanic("Security Exception"); }

status_t
init_cpu_traps(void)
{
    idt_maptrap(0, &fault_de);
    idt_maptrap(1, &fault_db);
    idt_mapint(2, &int_nomask);
    idt_maptrap(3, &fault_bp);
    idt_maptrap(4, &fault_of);
    idt_maptrap(5, &fault_br);
    idt_maptrap(6, &fault_ud);
    idt_maptrap(7, &fault_nm);
    idt_maptrap(8, &abort_df);
    idt_maptrap(10, &fault_ts);
    idt_maptrap(11, &fault_np);
    idt_maptrap(12, &fault_ss);
    idt_maptrap(13, &fault_gp); 
    idt_maptrap(14, &fault_pf);
    idt_maptrap(16, &fault_mf);
    idt_maptrap(17, &fault_ac);
    idt_maptrap(18, &abort_mc);
    idt_maptrap(19, &fault_xmxf);
    idt_maptrap(20, &fault_ve);
    idt_maptrap(30, &fault_sx);

    return STATUS_OK;
}
