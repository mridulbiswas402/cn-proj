/* Binding a AF_INET socket with address */
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


char * socket_addr(int s, char *buf, size_t bufsize){
    /* function to determine address of socket s
    address returned as string.*/
    struct sockaddr_in adr_inet;
    int len_inet = sizeof(adr_inet);

    // obtaining address of the socket
    int z = getsockname(s,(struct sockaddr *)&adr_inet,&len_inet);

    if(z==-1) return NULL;

    // converting address in to string 

    snprintf(buf,bufsize,"%s:%u",inet_ntoa(adr_inet.sin_addr),
    (unsigned)htons(adr_inet.sin_port));

    return buf;

}

int main(){

    /* create an IPv4 internet socket. */
    int sck_inet = socket(AF_INET,SOCK_STREAM,0);

    if(sck_inet == -1) bail("socket()");

    /* create an AF_INET address. */
    struct sockaddr_in adr_inet;

    // addr initialization.
    memset(&adr_inet,0,sizeof(adr_inet));

    adr_inet.sin_family=AF_INET;
    adr_inet.sin_port=ntohs(9000);
    
    if(!inet_aton("127.0.0.24",&adr_inet.sin_addr))
        bail("bad address");

    int len_inet = sizeof(adr_inet);

    // now binding the addr to the socket.

    int z=bind(sck_inet,(struct sockaddr *)&adr_inet,len_inet);

    if(z==-1) bail("bind()");

    char buf[64];

    if(!socket_addr(sck_inet,buf,sizeof(buf))) bail("socket_addr()");

    printf("Address %s\n",buf);


    close(sck_inet);
}