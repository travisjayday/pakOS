#ifndef PCI_H
#define PCI_H

#include "kernel.h"
#include "virt-mm.h"
#include "kstdio.h"

#define PCI_CFGADDR         0xcf8
#define PCI_CFGDATA         0xcfc
#define PCI_MULTIFUNC       0x80
#define PCI_ENUM_ALL_SLOTS  0xff

struct 
gpci_dev 
{
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint8_t  header_typ; 
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t mainclass;
};

#include "intel_8254x.h"

extern uint32_t gpci_devs_n;
extern struct gpci_dev** gpci_devs; 

status_t init_pci(void);
status_t load_pci_drivers();

uint32_t pci_cfg_rdword(struct gpci_dev* dev, uint8_t reg_offset);
uint16_t pci_cfg_rword(struct gpci_dev* dev, uint8_t reg_offset);

void pci_cfg_wdword(struct gpci_dev* dev, uint8_t reg_offset, uint32_t val);


#endif
