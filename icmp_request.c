/* creating and sending an icmp packet.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>

#define ICMP_PKT_SIZE 64
#define TIMEOUT 2


static void bail(const char *on_what){
    perror(on_what);
    exit(1);
}

struct icmp_pkt{
    struct icmphdr hdr;
    char msg[ICMP_PKT_SIZE-sizeof(struct icmphdr)];
};

typedef struct icmp_pkt icmp_pkt;

// Calculating the Check Sum
unsigned short checksum(void *b, int len)
{   //issue in this function.
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
 
    while(len>1){
        sum += *buf++;
    }
    if(len == 1){
        unsigned short u=0;
        u = *(unsigned char*)buf;
        sum += u;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char **argv){
    int pid = getpid();
    printf("pid: %d\n",pid);
    int seq_no = 0;

    char msg[] = "Hello I am Mridul";

    // if(argc<3){
    //     printf("Usage %s <NO OF ICMP REQUEST> <DEST IP ADDRESS>\n",argv[0]);
    //     exit(1);
    // }
    // int msg_cnt = atoi(argv[1]);
    
    int fd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if(fd<0) bail("socket()");


    // setting timeout of recv setting
    struct timeval t_out;
    t_out.tv_sec = TIMEOUT;
    if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&t_out,sizeof(t_out))<0) 
        bail("setsockopt()");
    

    // creating an icmp packet.
    icmp_pkt pkt;
    memset(&pkt,0,sizeof(pkt));

    pkt.hdr.type = ICMP_ECHO;
    pkt.hdr.un.echo.id = pid;
    pkt.hdr.un.echo.sequence = seq_no;
    pkt.hdr.checksum = checksum(&pkt, sizeof(pkt));
    memcpy(pkt.msg,msg,sizeof(msg));

    sleep(1);

    // initializing destination ip address.
    struct sockaddr_in ping_addr;
    ping_addr.sin_family = AF_INET;
    ping_addr.sin_port = htons(0);
    ping_addr.sin_addr.s_addr = inet_addr("192.168.0.1");

    //sending icmp packet.
    if(sendto(fd, &pkt, sizeof(pkt), 0,
        (struct sockaddr*)&ping_addr,
        sizeof(ping_addr))<0);

    
    
    

    close(fd);
}