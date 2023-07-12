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

 

    /* create an AF_INET address for server */
    struct sockaddr_in srvr_adr;
    memset(&srvr_adr,0,sizeof(srvr_adr));
    srvr_adr.sin_family=AF_INET;
    srvr_adr.sin_port=htons(9090);
    srvr_adr.sin_addr.s_addr = inet_addr(srvr_addr);
    
    if(srvr_adr.sin_addr.s_addr == INADDR_NONE) bail("bad address");
        
    /* create a UDP socket to connect with server*/
    int s = socket(AF_INET,SOCK_DGRAM,0);

    if(s == -1) bail("socket()");

    char dgram[512]; // recv buffer
   
    // wait for request.
    struct sockaddr_in adr;
    
    while(1){
        /* Promting User for a date time formate. */
        fputs("\n Enter formate string: ",stdout);

        if(!fgets(dgram,sizeof(dgram),stdin)) break; // EOF

        int z = strlen(dgram);

        if(z>0 && dgram[--z]=='\n') dgram[z]=0; // stomp out newline.

        // sending formate string to server.

        z = sendto(s,  // socket to send result
        dgram,         // datagram result to send
        strlen(dgram), // data gram length
        0,
        (struct sockaddr *)&srvr_adr, // server address
        sizeof(srvr_adr));                  // server address length

        if(z<0) bail("sento(2)");

        // test if we asked server to shutdown.
        if(!strcasecmp(dgram,"QUIT")) break; // yes we quit to.


        /* wait for a response*/

        int x =sizeof(adr);
        z = recvfrom(s,
            dgram,
            sizeof(dgram),
            0,
            (struct sockaddr *)&adr,
            &x);

        if(z<0) bail("recvfrom(2)");

        // process the request
        dgram[z]=0; // null terminating the buffer
        
        // repost the result.

        printf("Report from %s port %u:\n\t'%s'\n",
        inet_ntoa(adr.sin_addr),
        (unsigned)ntohs(adr.sin_port),
        dgram);
    }

    close(s);
    putchar('\n');
    return 0;
}