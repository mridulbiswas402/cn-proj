/*estd a specific AT_INET address. */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/un.h>
#include<sys/stat.h>
#include<sys/types.h>

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

    const unsigned char IPno[]={127,0,0,23}; /* local loop back addr. */

    /* create an IPv4 internet socket. */
    int sck_inet=socket(AF_INET,SOCK_STREAM,0);

    if(sck_inet == -1) bail("socket()");

    /* create an AF_INET address. */
    struct sockaddr_in inet_addr;

    // addr initialization.
    memset(&inet_addr,0,sizeof(inet_addr));

    inet_addr.sin_family=AF_INET;
    inet_addr.sin_port=ntohs(9000);
    memcpy(&inet_addr.sin_addr.s_addr,IPno,4);
    //inet_addr.sin_addr.s_addr=ntohl(INADDR_ANY);

    int len_inet = sizeof(inet_addr);

    // now binding the addr to the socket.

    int z=bind(sck_inet,(struct sock_addr *)&inet_addr,len_inet);

    if(z==-1) bail("bind()");

    /* display all of our bound socket */
    //system("netstat -pa --tcp");
    system(" netstat -pa --tcp 2>/dev/null| sed -n '1,/^Proto/p;/a.out/p'");

    close(sck_inet);

    //printf("%u\n",inet_addr.sin_port);
    //printf("%x\n",inet_addr.sin_addr.s_addr);
}