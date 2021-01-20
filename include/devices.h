#ifndef DEVICES_H
#define DEVICES_H

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

struct 
netdev 
{
    void (*write_partial_packet)(uint8_t* data, uint16_t len, uint8_t eop);
    struct gpci_dev* pcidev;
    struct mac_addr* mac_addr;
    struct ip_addr* ip_addr;
};

#endif
