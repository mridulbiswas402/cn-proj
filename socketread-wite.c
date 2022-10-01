#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>

int main(int argc,char *argv[]){

    int s[2]; // pair of file descriptor.

    int z=socketpair(AF_LOCAL,SOCK_STREAM,0,s); // socket pair creation.

    if(z==-1){
        fprintf(stderr,"%s :socketpair(AF_LOCAL,SOCK_STREAM,0,s)\n",strerror(errno));
        return 1;
    }

    /* writing a message to socket s[1] */

    char * message="hi how are you";
    z=write(s[1],(char *)message,strlen(message));

    if(z==-1){
        fprintf(stderr,"%s :write(s[1],(char *)message,strlen(message))\n",strerror(errno));
        return 2;
    }

    /* read message from socket s[0] */
    char buff[20];
    z=read(s[0],buff,sizeof(buff));

    if(z==-1){
        fprintf(stderr,"%s :read(s[0],buff,sizeof(buff))\n",strerror(errno));
        return 3;
    }

    printf("message recv:%s\n",buff);

    close(s[1]);
    close(s[0]);
}