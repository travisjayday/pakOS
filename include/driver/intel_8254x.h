#ifndef INTEL_8254X_H
#define INTEL_8254X_H

#include "kernel.h"
#include "pci.h"
#include "virt-mm.h"

#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_ARP 0x0806

#define ARP_HTYPE_ETH   1
#define ARP_PTYPE_IPV4  0x0800
#define ARP_OP_REQ      1
#define ARP_OP_RPY      2

/*
 *  Steps:
 *      - Use PCI config to get BAR0 to find MMIO space 
 *      - Use EERD register in MMIO space to read from EEPROM
 */
status_t load_driver_eth_intel_8254x(struct gpci_dev* pcidev);

struct mac_addr {
    uint64_t bytes : 48; 
    uint16_t reserved;
    char* str; 
};

struct ip_addr {
    uint32_t bytes;
    char* str;
};

struct tdesc {
    uint64_t buffer : 64; 
    uint16_t length : 16;
    uint8_t cso : 8;
    uint8_t cmd : 8;
    uint8_t sta : 4;
    uint8_t rsv : 4;
    uint8_t css : 8;
    uint16_t special : 16;
};

struct eth_header {
    uint64_t dst: 48;
    uint64_t src: 48;
    uint16_t type: 16;
} __attribute__((packed));

struct arp_frame {
    uint16_t htype: 16;
    uint16_t ptype: 16;
    uint8_t  hlen: 8;
    uint8_t  plen: 8;
    uint16_t operation : 16;
    uint64_t sender_hwaddr : 48;
    uint32_t sender_paddr  : 32;
    uint64_t target_hwaddr : 48;
    uint32_t target_paddr  : 32;
} __attribute__((packed));

void add_tx_buffer(uint64_t buffer_addr, uint16_t len, uint8_t eop) ;

void netd_send_ipv4_arp_packet(struct mac_addr* mac_sender, struct ip_addr* ip_sender, 
        struct mac_addr* mac_target, struct ip_addr* ip_target, uint8_t operation);


#endif
