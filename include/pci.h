#ifndef PCI_H
#define PCI_H

#include "kernel.h"
#include "virt-mm.h"
#include "kstdio.h"
#include "netdev.h"
#include "intel_8254x.h"

#define PCI_CFGADDR         0xcf8
#define PCI_CFGDATA         0xcfc
#define PCI_MULTIFUNC       0x80
#define PCI_ENUM_ALL_SLOTS  0xff

extern uint32_t gpci_devs_n;
extern struct gpci_dev** gpci_devs; 

/* Possible PCI devices */
extern struct netdev* netcard1; 

status_t init_pci(void);
status_t load_pci_drivers();

uint32_t pci_cfg_rdword(struct gpci_dev* dev, uint8_t reg_offset);
uint16_t pci_cfg_rword(struct gpci_dev* dev, uint8_t reg_offset);

void pci_cfg_wdword(struct gpci_dev* dev, uint8_t reg_offset, uint32_t val);

void pci_enable_bus_mastering(struct gpci_dev* pcidev);
 


#endif
