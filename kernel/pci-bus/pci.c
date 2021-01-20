#include "pci.h"

struct gpci_dev** gpci_devs; 
uint32_t gpci_devs_n = 0;
struct netdev* netcard1; 

/* 
 * Internal function declarations - Used only in pci.c 
 */
void             pci_enumerate_devs(void);
void             pci_findall(uint8_t on_bus);
struct gpci_dev* pci_add_dev(uint8_t on_bus, uint8_t on_slot, uint8_t using_fun);
uint32_t         _pci_cfg_rdword(uint8_t bus, uint8_t slot, uint8_t fun, uint8_t reg_offset);
uint16_t         _pci_cfg_rword(uint8_t bus, uint8_t slot, uint8_t fun, uint8_t reg_offset);
status_t         load_intel_devdrv(struct gpci_dev* dev);

//===========================================================================//
 

inline uint16_t
pci_get_vendor(uint8_t on_bus, uint8_t on_slot, uint8_t using_fun) {
    return _pci_cfg_rword(on_bus, on_slot, using_fun, 0);
}

inline uint8_t
pci_get_htype(uint8_t on_bus, uint8_t on_slot, uint8_t using_fun) {
    return (uint8_t) _pci_cfg_rword(on_bus, on_slot, using_fun, 14);
}

uint32_t
_pci_cfg_rdword(uint8_t bus, uint8_t slot, uint8_t fun, uint8_t reg_offset) 
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
_pci_cfg_rword(uint8_t bus, uint8_t slot, uint8_t fun, uint8_t reg_offset)
{
    uint32_t dword = _pci_cfg_rdword(bus, slot, fun, reg_offset);
    // offset ends in 2 iff we're accessing the higher 16 bits, so shift it 
    return (uint16_t) (dword >> ((reg_offset & 2) * 8) & 0xffff);
}

/*
 * Public for reading PCI config space
 */
uint32_t
pci_cfg_rdword(struct gpci_dev* dev, uint8_t reg_offset) {
    return _pci_cfg_rdword(dev->bus, dev->slot, dev->func, reg_offset); 
}

uint16_t
pci_cfg_rword(struct gpci_dev* dev, uint8_t reg_offset) {
    return _pci_cfg_rword(dev->bus, dev->slot, dev->func, reg_offset); 
}

void
pci_cfg_wdword(struct gpci_dev* dev, uint8_t reg_offset, uint32_t val) 
{
    uint32_t cfg = 0x80000000;      // set enable bit
    cfg |= dev->bus << 16;          // set bus 
    cfg |= (dev->slot & 0x1f) << 11;// set device
    cfg |= (dev->func & 0x7) << 8;  // set function
    cfg |= reg_offset & 0xfc;       // specify offset to dword register in config space (must be muleiple of dword)
    ___outl(PCI_CFGADDR, cfg);
    ___outl(PCI_CFGDATA, val);
}

struct gpci_dev*
pci_add_dev(uint8_t on_bus, uint8_t on_slot, uint8_t using_fun) {
    uint16_t vendor_id = pci_get_vendor(on_bus, on_slot, using_fun);
    if (vendor_id != 0xffff) {
        uint16_t dev_id  = _pci_cfg_rword(on_bus, on_slot, using_fun, 2);
        uint32_t class_c = _pci_cfg_rdword(on_bus, on_slot, using_fun, 8) >> 8;
        uint8_t htype    = pci_get_htype(on_bus, on_slot, using_fun);

        printk("\t[%d:%d](%d) %x:%x (%x/%d)\n", (uint32_t) on_bus, 
                (uint32_t) on_slot,  (uint32_t) using_fun,
                (uint32_t) vendor_id, (uint32_t) dev_id, class_c,
                (uint32_t) htype);

        struct gpci_dev* pcidev = kmalloc(sizeof(struct gpci_dev));
        pcidev->bus = on_bus;
        pcidev->slot = on_slot;
        pcidev->func = using_fun;
        pcidev->vendor_id = vendor_id;
        pcidev->device_id = dev_id;
        pcidev->mainclass = class_c;
        pcidev->header_typ = htype; 
        gpci_devs[gpci_devs_n++] = pcidev; 
        return pcidev;
    }
    return NULL;
}

void
pci_findall(uint8_t on_bus) 
{
    printk("Devices on PCI Bus %d:\n", on_bus);
    // look through all slots on this bus
    for (uint8_t slot = 0; slot < 32; slot++) {

        // check and add if device exists on that slot
        struct gpci_dev* pcidev = pci_add_dev(on_bus, slot, 0); 

        // if device existed, check if its multifunction
        if (pcidev != NULL && pcidev->header_typ & PCI_MULTIFUNC) {
            // if its multifunction, add device on this bus, slot
            // and loop through all functions
            for (uint32_t func = 1; func < 8; func++) 
                pci_add_dev(on_bus, slot, func);
        }
    }
    printk("\n");
}

void 
pci_enumerate_devs(void)
{
    uint8_t root_htype = pci_get_htype(0, 0, 0);
    if (root_htype & PCI_MULTIFUNC) {
        // this is a multibus PCI root controller, each function 
        // corresponds to a sub bus
        for (uint8_t func = 0; func < 8; func++) {
            if (pci_get_vendor(0, 0, func) != 0xffff) {
                // func exists, so sub bus exists, so enumerate it
                pci_findall(func);
            }
        }
    }
    else {
        // there is only one bus
        pci_findall(0);     
    }
}

void
pci_enable_bus_mastering(struct gpci_dev* pcidev)
{
    // read command register
    uint32_t cmd = pci_cfg_rdword(pcidev, 0x4);

    // enable bus mastering
    cmd |= (1 << 2);

    // write comand register
    pci_cfg_wdword(pcidev, 0x4, cmd);


    uint32_t cmd_check = pci_cfg_rdword(pcidev, 0x4);

    if (!(cmd_check & (1 << 2))) {
        kpanic("Failed to enable bus mastering on PCI device"); 
    }
}

status_t
init_pci(void)
{
    // allocate a max of 32 devices
    gpci_devs = kmalloc(32 * sizeof(struct gpci_dev*));

    // populate devices by recusrively scanning the pci bus
    pci_enumerate_devs();

    // if no devices found, fail
    if (gpci_devs_n == 0) return STATUS_FAILED_INIT;

    return STATUS_OK;
}

status_t 
load_pci_drivers()
{
    status_t status;
    struct gpci_dev* dev;

    for (uint32_t i = 0; i < gpci_devs_n; i++) {
        dev = gpci_devs[i];
        switch (dev->vendor_id) {
            case 0x8086: 
                // intel vendor
                status = load_intel_devdrv(dev);
                break;
            default: 
                status = STATUS_NO_DRIVER;
        }

        if (status == STATUS_NO_DRIVER) {
            printk("No driver found for PCI[%d:%d](%d)...\n", 
                    (uint32_t) dev->bus, (uint32_t) dev->slot,
                    (uint32_t) dev->func);
        }
        else if (status != STATUS_OK) {
            printk("Failed to initialize driver for PCI[%d:%d](%d)...\n", 
                    (uint32_t) dev->bus, (uint32_t) dev->slot,
                    (uint32_t) dev->func);
        }
    }

    return STATUS_OK;
}

status_t 
load_intel_devdrv(struct gpci_dev* dev) 
{
    status_t status; 
    switch (dev->device_id) {
        case 0x1019: case 0x101A: case 0x1010: case 0x1015:
        case 0x1012: case 0x101D: case 0x1079: case 0x107A:
        case 0x107B: case 0x100F: case 0x1011: case 0x1026:
        case 0x1027: case 0x1028: case 0x1107: case 0x1112:
        case 0x1013: case 0x1018: case 0x1076: case 0x1077:
        case 0x1078: case 0x1017: case 0x1016: case 0x100E: 
            netcard1 = (struct netdev*) kmalloc(sizeof(struct netdev));
            status = load_driver_eth_intel_8254x(dev, &netcard1);
            break;
        default:
            status = STATUS_NO_DRIVER;
    }

    return status;
}
