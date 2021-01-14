#include "pci.h"

struct gpci_dev** gpci_devs; 

uint32_t
pci_cfg_rdword(uint8_t bus, uint8_t slot, uint8_t fun, uint8_t reg_offset) 
{
    uint32_t cfg = 0x80000000;      // set enable bit
    cfg |= bus << 16;               // set bus 
    cfg |= (slot & 0x1f) << 11;     // set device
    cfg |= (fun & 0x7) << 8;        // set function
    cfg |= reg_offset & 0xfc;       // specify offset to dword register in config space (must be muleiple of dword)
    ___outl(PCI_CFGADDR, cfg);
    return (uint32_t) ___inl(PCI_CFGDATA);
}

uint16_t
pci_cfg_rword(uint8_t bus, uint8_t slot, uint8_t fun, uint8_t reg_offset)
{
    uint32_t dword = pci_cfg_rdword(bus, slot, fun, reg_offset);
    // offset ends in 2 iff we're accessing the higher 16 bits, so shift it 
    return (uint16_t) (dword >> ((reg_offset & 2) * 8) & 0xffff);
}

void
pci_findall(uint8_t on_bus) 
{
    printk("Devices on PCI Bus %d:\n", on_bus);
    for (uint8_t i = 0; i < 32; i++) {
        uint16_t vendor_id = pci_cfg_rword(on_bus, i, 0, 0);
        if (vendor_id != 0xffff) {
            uint16_t device_id = pci_cfg_rword(on_bus, i, 0, 2);
            printk("\t[%d:%d] %x:%x\n", (uint32_t) on_bus, (uint32_t) i, 
                    (uint32_t) vendor_id, (uint32_t) device_id);
        }
    }
    printk("\n");
}

void 
pci_enumerate_devs(void)
{
    pci_findall(0);     
}

status_t
init_pci(void)
{
    gpci_devs = kmalloc(32 * sizeof(struct gpci_devs*));
    pci_enumerate_devs();
    return STATUS_OK;
}
