#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>	
#include<errno.h>

int main(int argc,char *argv[]){

    int s[2]; // pair of file descriptor.

    int z=socketpair(AF_LOCAL,SOCK_STREAM,0,s); // socket pair creation.

    if(z==-1){
        fprintf(stderr,"%s :socketpair(AF_LOCAL,SOCK_STREAM,0,s)\n",strerror(errno));
        return 1;
    }

	/* fork */
	pid_t chpid=fork();

	if(chpid==0){
		/*child process ->client */
		char rxbuf[80];
		printf("client pid :%ld\n",(long)getpid());

		/* writing a message to socket s[1] */
		char * message=":hello";
		z=write(s[1],(char *)message,strlen(message));

		if(z==-1){
			fprintf(stderr,"%s :write(s[1],(char *)message,strlen(message))\n",strerror(errno));
			return 2;
		}

		/* read message from socket s[0] */
		z=read(s[1],rxbuf,sizeof(rxbuf));

		if(z==-1){
			fprintf(stderr,"%s :read(s[0],buff,sizeof(buff))\n",strerror(errno));
			return 3;
		}

		printf("s[1] message recv:%s\n",rxbuf);
		exit(0);
		
	}
	else{
		char rxbuf[80];
		printf("server pid :%ld\n",(long)getpid());

		/* writing a message to socket s[1] */
		char * message=":hi";
		z=write(s[0],(char *)message,strlen(message));

		if(z==-1){
			fprintf(stderr,"%s :write(s[1],(char *)message,strlen(message))\n",strerror(errno));
			return 2;
		}	
		/* read message from socket s[0] */
		z=read(s[0],rxbuf,sizeof(rxbuf));

		if(z==-1){
			fprintf(stderr,"%s :read(s[0],buff,sizeof(buff))\n",strerror(errno));
			return 3;
		}

		printf("s[0] message recv:%s\n",rxbuf);
	}

    close(s[1]);
    close(s[0]);
	int status;
	waitpid(chpid,&status,0);
}