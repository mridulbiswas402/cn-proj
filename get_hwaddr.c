/* getting hardware address from network device demo
using ioctl() */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/un.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <sys/ioctl.h>

#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ether.h>

#include<netinet/in.h>
#include<arpa/inet.h>




#include<unistd.h>
#include<errno.h>

/* this function is for error reporting */
static void bail(const char *on_what){
    perror(on_what);
    exit(1);
}


int main(){

    // packet socket
    int sockfd = socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_802_3));
    if(sockfd<0) bail("socket()");

    // getting hardware address.
    struct ifreq ethreq;
    strncpy(ethreq.ifr_name, "eno1", IF_NAMESIZE);

    if(ioctl(sockfd,SIOCGIFHWADDR, &ethreq) == -1) bail("ioctl()"); 

    printf("%x:%x:%x:%x:%x:%x",ethreq.ifr_hwaddr.sa_data[0],
    ethreq.ifr_hwaddr.sa_data[1],
    ethreq.ifr_hwaddr.sa_data[2],
    ethreq.ifr_hwaddr.sa_data[3],
    ethreq.ifr_hwaddr.sa_data[4],
    ethreq.ifr_hwaddr.sa_data[5]);

  
}