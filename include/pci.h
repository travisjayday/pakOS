#ifndef PCI_H
#define PCI_H

#include "kernel.h"
#include "virt-mm.h"
#include "kstdio.h"

#define PCI_CFGADDR 0xcf8
#define PCI_CFGDATA 0xcfc

struct 
gpci_dev 
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t mainclass;
    uint32_t subclass;
    uint8_t  header_typ; 
};

extern struct gpci_dev** gpci_devs; 

status_t init_pci(void);


#endif
