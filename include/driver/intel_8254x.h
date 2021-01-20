#ifndef INTEL_8254X_H
#define INTEL_8254X_H

#include "kernel.h"
#include "pci.h"

/*
 *  Steps:
 *      - Use PCI config to get BAR0 to find MMIO space 
 *      - Use EERD register in MMIO space to read from EEPROM
 */
status_t load_driver_eth_intel_8254x(struct gpci_dev* pcidev, struct netdev** netdev);

struct tdesc {
    uint64_t buffer : 64; 
    uint16_t length : 16;
    uint8_t cso : 8;
    uint8_t cmd : 8;
    uint8_t sta : 4;
    uint8_t rsv : 4;
    uint8_t css : 8;
    uint16_t special : 16;
} __attribute__((packed));

void add_tx_buffer(uint8_t* buffer_addr, uint16_t len, uint8_t eop); 

#endif
