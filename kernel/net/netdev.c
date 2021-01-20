#include "netdev.h"

#define ble16(big) ((uint16_t) big >> 8) | ((uint16_t) (big << 8))
#define ble32(big) (((0x000000ff & big) << 24) | ((0x0000ff00 & big) << 8) | \
                    ((0x00ff0000 & big) >> 8)  | ((0xff000000 & big) >> 24))

struct eth_header*
netd_make_eth2_header(struct mac_addr* src, struct mac_addr* dst, 
        uint16_t eth_type, uint16_t payload_size) 
{
    struct eth_header* head = kmalloc(sizeof(struct eth_header) + payload_size);  
    head->dst = dst->bytes;
    head->src = src->bytes;
    head->type = ble16(eth_type);
    return head;
}

void
netd_send_ipv4_arp_packet(struct netdev* dev, 
        struct mac_addr* mac_target, struct ip_addr* ip_target, uint16_t operation) 
{
    struct mac_addr eth_target; 
    
    // broadcast mac if we're requetsing
    if (operation == ARP_OP_REQ) {
        eth_target.bytes = 0xffffffffffff;
    }

    struct eth_header* head = netd_make_eth2_header(
            dev->mac_addr, &eth_target, ETH_TYPE_ARP, sizeof(struct arp_frame));

    struct arp_frame* frame = (struct arp_frame*) (head + 1); 

    frame->htype = ble16(ARP_HTYPE_ETH);
    frame->ptype = ble16(ARP_PTYPE_IPV4);
    frame->hlen  = 6;
    frame->plen  = 4;
    frame->operation = ble16(operation);
    frame->sender_hwaddr = dev->mac_addr->bytes;
    frame->sender_paddr  = dev->ip_addr->bytes;
    frame->target_hwaddr = mac_target->bytes;
    frame->target_paddr  = ip_target->bytes;

    dev->write_partial_packet((uint8_t*) head, 
            sizeof(struct eth_header) + sizeof(struct arp_frame), 1);
}


