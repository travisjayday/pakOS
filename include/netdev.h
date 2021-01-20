#ifndef NETDEV_H
#define NETDEV_H

#include "kernel.h"
#include "netdev.h"

#define ETH_TYPE_IPV4 0x0800ul
#define ETH_TYPE_ARP 0x0806ul

#define ARP_HTYPE_ETH   1
#define ARP_PTYPE_IPV4  0x0800
#define ARP_OP_REQ      1
#define ARP_OP_RPY      2

struct mac_addr {
    uint64_t bytes : 48; 
    uint16_t reserved;
    char* str; 
};

struct ip_addr {
    uint32_t bytes;
    char* str;
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

void netd_send_ipv4_arp_packet(struct netdev* dev, struct mac_addr* mac_target, struct ip_addr* ip_target, uint16_t operation);


#endif
