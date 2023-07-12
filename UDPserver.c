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

#include<time.h>

/* this function is for error reporting */
static void bail(const char *on_what){
    fputs(strerror(errno),stderr);
    fputs(": ",stderr);
    fputs(on_what,stderr);
    fputc('\n',stderr);
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

int main(int argc, char **argv){

    
    /* Providing a server address from cmd line
    else assigning a default loop back address.*/
    char * srvr_addr = NULL;
    if(argc>=2){
        srvr_addr = argv[1];
    }
    else{
        srvr_addr = "127.0.0.23";
    }

    /* create a UDP socket */
    int s = socket(AF_INET,SOCK_DGRAM,0);

    if(s == -1) bail("socket()");

    /* create an AF_INET address for server */
    struct sockaddr_in adr_inet;
    memset(&adr_inet,0,sizeof(adr_inet));
    adr_inet.sin_family=AF_INET;
    adr_inet.sin_port=htons(9090);
    adr_inet.sin_addr.s_addr = inet_addr(srvr_addr);
    
    if(adr_inet.sin_addr.s_addr == INADDR_NONE) bail("bad address");
        

    int len_inet = sizeof(adr_inet);

    /*now binding the addr to the socket so that 
    client program can contact this server.*/

    int z=bind(s,(struct sockaddr *)&adr_inet,len_inet);

    if(z==-1) bail("bind()");

    char dgram[512]; // recv buffer
    char dtfmt[512]; // date time result
    time_t td; // current time and date.
    struct tm tm; // time value

    // wait for request.
    struct sockaddr_in adr_clnt;
    
    while(1){
        len_inet = sizeof(adr_clnt);
        z = recvfrom(s,         // socket
                dgram,          // receiving buffer
                sizeof(dgram),  // max recv buffer size
                0,
                (struct sockaddr *)&adr_clnt,  // client address
                &len_inet);
        if(z<0) bail("recvfrom(2)");

        // process the request
        dgram[z]=0; // null terminating the buffer
        if(!strcasecmp(dgram,"QUIT")) break; /* quit server if QUIT is recieved*/

        // getting current date and time.

        time(&td); //get current time and date
        tm = *localtime(&td); // broken time components.

        /*formate a new time and date string
        based on input formate string */

        strftime(dtfmt, // formated result
        sizeof(dtfmt),
        dgram,          // input date/time formate
        &tm);           // input date/time value

        // sending the formated result back to the client.
        z = sendto(s,   // socket to semd result
        dtfmt,         // datagram result to send
        strlen(dtfmt), // datagram length
        0,
        (struct sockaddr *)&adr_clnt, // client address 
        len_inet);   // client address length

        if(z<0) bail("sendto(2)");

    }

    close(s);
    return 0;
}