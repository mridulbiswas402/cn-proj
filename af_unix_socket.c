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
    /* create a AF_UNIX socket */       
    int unix_skt=socket(AF_UNIX,SOCK_STREAM,0);   

    if(unix_skt==-1) bail("socket()");

    const char unix_pth[]="/tmp/cmater_socket"; // pathname.
    /* removes pathname from socket incase it exits from prior run */
    unlink(unix_pth);  

    struct sockaddr_un adr_unix;
    memset(&adr_unix,0,sizeof(adr_unix));

    /* socket initialization. */
    adr_unix.sun_family=AF_UNIX;
    
    strncpy(adr_unix.sun_path,unix_pth,sizeof(adr_unix.sun_path)-1)
    [sizeof(adr_unix.sun_path)-1]=0;

    int len_unix=SUN_LEN(&adr_unix);

    /* binding the address to socket */

    int z=bind(unix_skt,(struct sockaddr *)&adr_unix,len_unix);

    if(z==-1) bail("bind()");

    /* display all of our bound socket */
    //system("netstat -pa --unix");
    system(" netstat -pa --unix 2>/dev/null| sed -n '/^Active UNIX/,/^Proto/p;/a.out/p'");

    close(unix_skt);
    unlink(unix_pth);
     return 0;
}