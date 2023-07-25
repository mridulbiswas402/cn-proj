/* this program captures one ip packet from ethernet frame 
and prints usefull fields from the ip header */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<linux/if_packet.h>
#include<net/ethernet.h>
#include<net/if.h>
#include<netinet/ether.h>
#include<netinet/ip.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>


int main(void) {
    int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
    if (fd < 0)
    {
        perror("cannot open socket");
    }

    // address for this host.
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = if_nametoindex("eno1");
    sll.sll_protocol = htons(ETH_P_ALL);

    // binding address with socket.
    if (bind(fd, (struct sockaddr*)&sll, sizeof(sll)) < 0)
    {
        perror("cannot bind socket to device");
        return 1;
    }

    char recvBuf[2048];
    
    struct sockaddr_ll saddr; // place to hold src_addr
    socklen_t saddr_size = sizeof(saddr);

    /*src_addr :The src_addr parameter is a pointer to a sockaddr structure that you 
        allocate and will be filled in by recvfrom to identify the sender of 
        the message. The length of this structure will be stored in src_len */

    size_t len = recvfrom(fd, recvBuf, 1518, 0, (struct sockaddr*)&saddr, &saddr_size);

    recvBuf[len]=0; // null terminating.
    //printf("bytes recieved = %ld: %s\n",len,recvBuf);
    
    struct ip *ip_hdr;

    ip_hdr = (struct ip *)recvBuf;

    int hdr_len = ip_hdr->ip_hl;
    int pkt_len = ntohs(ip_hdr->ip_len);
    int ip_proto = ip_hdr->ip_p;

    if(ip_hdr->ip_v != 4) printf("unknown ip version\n");

    /* see if we have as much packet as we should */
    if(len < pkt_len)
        printf("\ntruncated IP - %ld bytes missing\n",pkt_len - len);
        

    printf("hdr_len: %d, pkt_len: %d, ip_proto: %d\n",hdr_len,pkt_len,ip_proto);
    fprintf(stdout,"IP: ");
    fprintf(stdout,"%s ",inet_ntoa(ip_hdr->ip_src));
    fprintf(stdout,"-> %s\n",inet_ntoa(ip_hdr->ip_dst));

    close(fd);

    return 0;
}
