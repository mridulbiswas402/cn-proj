/* Sending an arp message. */

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>  //htons etc
#include <errno.h>


struct arp_pkt{
    u_int16_t hw_t;
    u_int16_t proto_t;
    u_int8_t hw_l;
    u_int8_t proto_l;
    u_int16_t opcode; // 1 : request, 2 : reply
    u_int8_t src_mac[ETH_ALEN];
    in_addr_t src_ip;
    u_int8_t dst_mac[ETH_ALEN];
    in_addr_t dst_ip;
} __attribute__ ((__packed__));

typedef struct arp_pkt arp_pkt;

/* this function is for error reporting */
static void bail(const char *on_what){
    perror(on_what);
    exit(1);
}

int get_if_info(int sockfd, const char *ifname, uint32_t *ip, u_int8_t *mac, int *ifindex);



int main(void) {
    uint32_t dst_ip = inet_addr("192.168.0.1");
    char *ifname = "eno1";
    uint32_t src_ip;
    u_int8_t src_mac[ETH_ALEN];
    int ifindex;
    
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (fd < 0) bail("socket()");

    if(get_if_info(fd,ifname,&src_ip,src_mac,&ifindex)<0) 
        printf("fail to retrieve interface information\n");

    
    // address for this host.
    struct sockaddr_ll my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sll_family   = AF_PACKET;
    my_addr.sll_ifindex  = ifindex;
    my_addr.sll_protocol = htons(ETH_P_ARP);

    // binding address with socket.
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) bail("bind()");
   

    /* creating a broadcast address*/
    struct sockaddr_ll socket_address;
    memset(&socket_address,0,sizeof(socket_address));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_hatype = htons(ARPHRD_ETHER);
    socket_address.sll_pkttype = (PACKET_BROADCAST);
    socket_address.sll_halen = ETH_ALEN;
    

    /* creating ethernet packet*/
    unsigned char buffer[128];
    memset(buffer, 0, sizeof(buffer));

    struct ethhdr *frame = (struct ethhdr *) buffer;
    arp_pkt *rqst = (arp_pkt *) (buffer + ETH_HLEN);

    //Broadcast
    memset(frame->h_dest, 0xff, ETH_ALEN);
    memcpy(frame->h_source,src_mac, ETH_ALEN);
    frame->h_proto = htons(ETH_P_ARP);

    /* creating an arp request packet*/
    memset(rqst,0x00,sizeof(arp_pkt));
    rqst->hw_t = htons(ARPHRD_ETHER); // if_arp.h
    rqst->proto_t = htons(ETH_P_IP);  // if_ether.h
    rqst->hw_l = ETH_ALEN;
    rqst->proto_l = sizeof(in_addr_t);
    rqst->opcode = htons(ARPOP_REQUEST);
    memcpy(rqst->src_mac,src_mac,ETH_ALEN);
    rqst->src_ip = src_ip;
    rqst->dst_ip = dst_ip;

    // sending arp message this can be capture using wireshark.
    size_t len = sendto(fd,
        buffer,
        52,
        0,
        (struct sockaddr *)&socket_address,
        sizeof(socket_address));

    if(len<=0) bail("sendto()");

    close(fd);
    return 0;
}

int get_if_info(int sockfd, const char *ifname, uint32_t *ip, u_int8_t *mac, int *ifindex)
{
    /*
    * Gets interface information by name:
    * IPv4
    * MAC
    * ifindex
    * return 0 on success. else -1;
    */
    printf("get_if_info for %s\n", ifname);
    
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));

    if(strlen(ifname) > (IFNAMSIZ - 1)) {
        printf("Too long interface name, MAX=%i\n", IFNAMSIZ - 1);
        return -1;
    }

    strcpy(ifr.ifr_name, ifname);

    //Get interface index using name
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        return -1;
    }
    *ifindex = ifr.ifr_ifindex;
    printf("interface index is %d\n", *ifindex);

    //Get MAC address of the interface
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        return -1;
    }

    //Copy mac address to output
    memcpy(mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    printf("MAC : %x:%x:%x:%x:%x:%x\n",mac[0],mac[1],
            mac[2],mac[3],mac[4],mac[5]);

    //Get IP address of the interface
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1) {
        perror("SIOCGIFADDR");
        return -1;
    }

    if(ifr.ifr_addr.sa_family != AF_INET){
        printf("Not AF_INET\n");
        return -1;
    }
    struct sockaddr_in *ptr = (struct sockaddr_in *)&ifr.ifr_addr;
    *ip = ptr->sin_addr.s_addr;
    printf("IP : %s\n",inet_ntoa(ptr->sin_addr));
    return 0;
}

/* after compilation we need to set capabilities of the executable file
as follows:
sudo setcap 'cap_net_raw+epi cap_net_admin+epi' a.out

to be able to succesfully run the program.
*/