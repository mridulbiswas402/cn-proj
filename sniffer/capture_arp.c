/* this program captures all arp packet from a given interface,
and prints usefull fields from it*/

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
#include <arpa/inet.h> //htons etc
#include <errno.h>

#define BUF_SIZE 60

struct arp_pkt
{
    u_int16_t hw_t;
    u_int16_t proto_t;
    u_int8_t hw_l;
    u_int8_t proto_l;
    u_int16_t opcode; // 1 : request, 2 : reply
    u_int8_t src_mac[ETH_ALEN];
    in_addr_t src_ip;
    u_int8_t dst_mac[ETH_ALEN];
    in_addr_t dst_ip;
} __attribute__((__packed__));

typedef struct arp_pkt arp_pkt;

/* this function is for error reporting */
static void bail(const char *on_what)
{
    perror(on_what);
    exit(1);
}

int get_if_info(int sockfd, const char *ifname, uint32_t *ip, u_int8_t *mac, int *ifindex);

void print_mac(unsigned char *);

int main(int argc, char **argv)
{
    if(argc<2){
        printf("Usage: %s <INTERFACE>\n", argv[0]);
        return 1;
    }

    char *ifname = argv[1];
    uint32_t src_ip;
    u_int8_t src_mac[ETH_ALEN];
    int ifindex;

    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
        bail("socket()");

    if (get_if_info(fd, ifname, &src_ip, src_mac, &ifindex) < 0)
        printf("fail to retrieve interface information\n");

    // address for this host.
    struct sockaddr_ll my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sll_family = AF_PACKET;
    my_addr.sll_ifindex = ifindex;
    my_addr.sll_protocol = htons(ETH_P_ALL);

    // binding address with socket.
    if (bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
        bail("bind()");

    unsigned char recvbuf[BUF_SIZE]; // buffer to recieve incoming packets.

    while (1)
    {
        memset(recvbuf,0,BUF_SIZE);
        int len = recvfrom(fd, recvbuf, BUF_SIZE, 0, NULL, NULL);
        if (len < 0)
            bail("recvfrom()");

        // getting information from ethernet header.

        struct ethhdr *frame = (struct ethhdr *)recvbuf;
        arp_pkt *arp = (arp_pkt *)(recvbuf + ETH_HLEN);

        if(ntohs(frame->h_proto) == ETH_P_ARP)
        {
            printf("\n%d bytes recieved\n", len);

            printf("/*--- Ethernet ---*/\n");
            printf("src mac: ");
            print_mac(frame->h_source);
            printf("dst mac: ");
            print_mac(frame->h_dest);

            // getting information from arp pkt.
            printf("/*--- ARP ---*/\n");
            printf("arp opcode: %d\n",ntohs(arp->opcode));
            struct in_addr ip_adr;
            memset(&ip_adr, 0, sizeof(struct in_addr));
            ip_adr.s_addr = arp->src_ip;
            printf("src IP: %s, ", inet_ntoa(ip_adr));
            printf("src mac: ");
            print_mac(arp->src_mac);
            ip_adr.s_addr = arp->dst_ip;
            printf("dst IP: %s, ", inet_ntoa(ip_adr));
            printf("dst mac: ");
            print_mac(arp->dst_mac);

            printf("------------------------------\n");

        }
    }

    close(fd);
    return 0;
}

void print_mac(unsigned char *mac)
{
    printf("%x:%x:%x:%x:%x:%x\n", mac[0],
           mac[1],
           mac[2],
           mac[3],
           mac[4],
           mac[5]);
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

    if (strlen(ifname) > (IFNAMSIZ - 1))
    {
        printf("Too long interface name, MAX=%i\n", IFNAMSIZ - 1);
        return -1;
    }

    strcpy(ifr.ifr_name, ifname);

    // Get interface index using name
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1)
    {
        perror("SIOCGIFINDEX");
        return -1;
    }
    *ifindex = ifr.ifr_ifindex;
    printf("interface index is %d\n", *ifindex);

    // Get MAC address of the interface
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
    {
        perror("SIOCGIFINDEX");
        return -1;
    }

    // Copy mac address to output
    memcpy(mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    printf("MAC : %x:%x:%x:%x:%x:%x\n", mac[0], mac[1],
           mac[2], mac[3], mac[4], mac[5]);

    // Get IP address of the interface
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1)
    {
        perror("SIOCGIFADDR");
        return -1;
    }

    if (ifr.ifr_addr.sa_family != AF_INET)
    {
        printf("Not AF_INET\n");
        return -1;
    }
    struct sockaddr_in *ptr = (struct sockaddr_in *)&ifr.ifr_addr;
    *ip = ptr->sin_addr.s_addr;
    printf("IP : %s\n", inet_ntoa(ptr->sin_addr));
    return 0;
}