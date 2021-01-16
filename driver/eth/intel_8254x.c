#include "intel_8254x.h"
#include "pci.h"

uint32_t* regspace;
struct tdesc* txqueue;
uint32_t txqueue_n = 64;    // txqueue_n * 2 bytes must be a multiple of 128 bytes
struct tdesc* txtail;

#define BAR0_SIZE           128 * 1024  // Register space is 128KB big

#define REG_EERD            0x14
#define REG_STATUS          0x8 

/* Transmission Registers */
#define REG_TCTL            0x400
#define REG_TDBAL           0x3800
#define REG_TDBAH           0x3804
#define REG_TDLEN           0x3808
#define REG_TDT             0x3818
#define REG_TDH             0x3810
#define REG_TSCTFC          0x40fc

/* Stats */
#define REG_GPTC            0x4080      // Good packets transmitted count 
#define REG_TOTL            0x40c8      // Total octets transmitted (lower) 
#define REG_TOTH            0x40cc      // Total octets transmitted (lower) 

#define EEPROM_MAC          0x0         // MAC address is 6 bytes long
#define EEPROM_VENDOR       0xE   

#define EEPROM_IPV4_LANA    0x25        // IP addr is 4 bytes long
#define EEPROM_IPV4_LANB    0x15
#define EEPROM_MGMNT_LANA   0x23        // Management Control Register
#define EEPROM_MGMNT_LANB   0x13        // Management Control Register

#define LAN_A               0xA
#define LAN_B               0xB



#define rreg(reg_offset)     \
    *((uint32_t*) ((uint8_t*) regspace + reg_offset))
#define wreg(reg_offset, v)                      \
    *((uint32_t*) ((uint8_t*) regspace + reg_offset)) = v

uint16_t eeprom_rword(uint8_t offset) {
    uint32_t cmd = 1 | (offset << 8);
    wreg(REG_EERD, cmd);
    uint32_t resp;
    do {
        resp = rreg(REG_EERD);
        printk("");
    } while ((resp & (1 << 4)) == 0);
    return (uint16_t) (resp >> 16); 
}

struct mac_addr* 
get_mac_addr(void) 
{
    // reading MAC from left to right, these are the bytes
    uint16_t mac21 = eeprom_rword(EEPROM_MAC);
    uint16_t mac43 = eeprom_rword(EEPROM_MAC + 1);
    uint16_t mac65 = eeprom_rword(EEPROM_MAC + 2);

    uint8_t* mac = kmalloc(6);
    mac[0] = mac21 & 0xff;
    mac[1] = mac21 >> 8;
    mac[2] = mac43 & 0xff;
    mac[3] = mac43 >> 8;
    mac[4] = mac65 & 0xff;
    mac[5] = mac65 >> 8;

    struct mac_addr* addr = kmalloc(sizeof(struct mac_addr));
    addr->bytes = *((uint64_t*) mac);

    char* str = kmalloc(9);
    uint8_t c = 0;
    for (uint8_t i = 0; i < 6; i++) {
        char* s = itoa(mac[i], 16);
        str[c++] = s[0];
        str[c++] = s[0] != '0'? s[1] : '0';
        str[c++] = i != 5 ? ':' : '\0';
        kfree(s);
    }
    addr->str = str;

    return addr;
}

// Offset is either EEPROM_IPV4_LANA or EEPROM_IPV4_LANB
struct ip_addr*
get_ipv4_addr(uint8_t lan_type)
{
    uint8_t mgmt_offset = lan_type == LAN_A ? EEPROM_MGMNT_LANA : EEPROM_MGMNT_LANB;
    uint16_t mgmt = eeprom_rword(mgmt_offset);
    if (!(mgmt & (1 << 6))) {
        // ipv4 is not valid 
        return NULL;
    }
    uint8_t ip_offset = lan_type == LAN_A? EEPROM_IPV4_LANA : EEPROM_IPV4_LANB;
    uint16_t ip12 = eeprom_rword(ip_offset);
    uint16_t ip34 = eeprom_rword(ip_offset + 1);
    uint32_t ip = (((uint32_t) ip34) << 16) | ip12;

    char* str = kmalloc(15);
    /*uint8_t c = 0;
    for (uint8_t i = 0; i < 15; i++) {
        char* s = itoa(mac[i], 16);
        str[c++] = s[0];
        str[c++] = s[0] != '0'? s[1] : '0';
        str[c++] = i != 5 ? ':' : '\0';
        kfree(s);
    }*/
    
    struct ip_addr* addr = kmalloc(sizeof(struct ip_addr));
    addr->bytes = ip;
    addr->str = str;
    return addr;
}

struct eth_header*
netd_make_eth2_header(struct mac_addr* src, struct mac_addr* dst, 
        uint16_t eth_type, uint16_t payload_size) 
{
    struct eth_header* head = kmalloc(sizeof(struct eth_header) + payload_size);  
    head->dst = dst->bytes;
    head->src = src->bytes;
    head->type = eth_type;
    return head;
}

void
netd_send_ipv4_arp_packet(struct mac_addr* mac_sender, struct ip_addr* ip_sender, 
        struct mac_addr* mac_target, struct ip_addr* ip_target, uint8_t operation) 
{
    struct mac_addr eth_target; 
    
    // broadcast mac if we're requetsing
    if (operation == ARP_OP_REQ) {
        eth_target.bytes = 0xffffff;
    }

    struct eth_header* head = netd_make_eth2_header(
            mac_sender, &eth_target, ETH_TYPE_ARP, sizeof(struct arp_frame));

    struct arp_frame* frame = (struct arp_frame*) (head + 1); 

    frame->htype = ARP_HTYPE_ETH;
    frame->ptype = ARP_PTYPE_IPV4;
    frame->hlen  = 6;
    frame->plen  = 4;
    frame->operation = operation;
    frame->sender_hwaddr = mac_sender->bytes;
    frame->sender_paddr  = ip_sender->bytes;
    frame->target_hwaddr = mac_target->bytes;
    frame->target_paddr  = ip_target->bytes;

    add_tx_buffer((uint64_t)((uintptr_t) head), 
            sizeof(struct eth_header) + sizeof(struct arp_frame), 1);
}

void
add_tx_buffer(uint64_t buffer_addr, uint16_t len, uint8_t eop) 
{
    struct tdesc* last = txtail;
    
    // if the last descriptor is valid and has not been processed yet, wait
    // Checks TDESC.STA.DD bit to check if descriptor has been consumed by hardware. 
    while (last->buffer != 0 && (last->sta & 0x1) == 0) printk("");

    // overwrite the last dsecriptor with the current one
    struct tdesc* current = last;
    current->buffer = _lpa(buffer_addr);  // physical address of buffer
    current->length = len;          // length of buffer
    current->cso    = 0;            // checksum offset 0 (not used)

    uint8_t cmd = 0;

    // CMD.IDE (Bit 7) 
    cmd &= ~(1 << 7);       // disable IDE (this makes it so that transmit
                            // interrupt always fires if RS is set
    // CMD.VLE (Bit 6)
    cmd &= ~(1 << 6);       // disbale VLAN packet because this is not VLAN 

    // CMD.DEXT (Bit 5)
    cmd &= ~(1 << 5);       // enable legacy TDESC mode to get right struct layout  

    // CMD.RPS (Bit 4)
    cmd &= ~(1 << 4);       // only used with 82544GC/EI for packet sent reports

    // CMD.RS (Bit 3)
    cmd |= 1 << 3;          // enable report status. Sets TDESC.STA.DD when tdesc
                            // has been consumed by hardware and can be re-used
    // CMD.IC (Bit 2)
    cmd &= ~(1 << 2);       // Do not insert checksum. we manually compute it.

    if (eop) {
        // CMD.IFCS (Bit 1)
        cmd |= 1 << 1;     // Insert Frame Check / Cyclic Redundancy check on 
                           // ethernet frame transmisison. I don't think we need? 
        // CMD.EOP (Bit 0)
        cmd |= 1 << 0;     // This discriptor is the last descriptor making up a packet. 
    }

    current->cmd = cmd; 
    current->sta = 0;       // hardware will modify the descriptor status
    current->rsv = 0;       // reserved 
    current->css = 0;       // checksum start (we don't use this)
    current->special = 0;   // not using 802.1q/802.1ac tagging info

    txtail++; 

    // wrap around
    if ((uint32_t) (txtail - txqueue) > txqueue_n) txtail = txqueue; 
}

status_t
load_driver_eth_intel_8254x(struct gpci_dev* pcidev)
{
    // 0xd0007038:     0xaaaabbbbccccdddd      0xeeee5574432211ff
    uint8_t* noalign = kcalloc(16 + txqueue_n * sizeof(struct tdesc), 1);
    // 16 byte align txqueue
    txqueue = (struct tdesc*) (((uintptr_t) noalign + 16) & ~0xf);
    txtail = txqueue;

    printk("%x <-- %x\n", txqueue, _lpa(txqueue));
    printk("Loading driver for 8254x NIC...\n");

    // map BAR0 into vaspace
    uint32_t hwbar0 = pci_cfg_rdword(pcidev, 0x10);

    // bit set if Bar0 is actually a 64 bit address
    if (hwbar0 & 0x2) {
        // unimplemented
        return STATUS_DRIVER_ERROR; 
    }

    // clear flag bits
    hwbar0 &= 0xfffffff8;

    // map bar0 into virtual address space 
    regspace = maphwdev(hwbar0, BAR0_SIZE / 4096);

    uint16_t vendor = eeprom_rword(EEPROM_VENDOR);
    printk("Vendor: %x\n", (uint32_t) vendor); 

    struct mac_addr* mac = get_mac_addr();
    (void) mac;
    printk("Mac address is %s\n", mac->str);

    struct ip_addr* ip = get_ipv4_addr(LAN_A);
    if (ip == NULL || ip->bytes == 0xffffffff) {
        printk("No valid IP Address in EEPROM!!\n");
    }
    else {
        printk("Ipv4: %x\n", ip->bytes);
    }

    struct mac_addr mac_target; 
    mac_target.bytes = 0;

    struct ip_addr ip_target;
    ip_target.bytes = 0x123456;

    netd_send_ipv4_arp_packet(mac, ip, &mac_target, &ip_target, ARP_OP_REQ);

    // set trasmist descriptor base
    uintptr_t tdaddr = _lpa(txqueue);
    if ((tdaddr & 0x7) != 0) {
        kpanic(itoa(tdaddr, 16));
    }
    wreg(REG_TDBAL, tdaddr);
    wreg(REG_TDBAH, 0x0);
    wreg(REG_TDLEN, txqueue_n * 2);
    wreg(REG_TDH, 0x0);
    wreg(REG_TDT, txtail - txqueue);

    printk("Tail sis at %d", rreg(REG_TDT));
    

    // enable transmission
    uint32_t tctl = rreg(REG_TCTL);
    tctl |= 1 << 1; 
    wreg(REG_TCTL, tctl);

    for (int i = 0; i < 100; i++) printk("");

    printk("Head sis at %d", rreg(REG_TDH));
    printk("staus %lx %lx\n", *((uint64_t*) txqueue), *((uint64_t*) txqueue + 1));
    printk("Transmitted; %d\n", rreg(REG_TOTL));
    printk("Failed; %d\n", rreg(REG_TSCTFC));
    
    return STATUS_OK;
}
