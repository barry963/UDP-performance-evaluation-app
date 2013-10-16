/*receiverprog.c - a server, datagram sockets*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>

/* the port users will be connecting to */
#define MYPORT 8888
#define MAXBUFLEN 501
unsigned char mode_para[2]="u";

int execute;
/*Capture the Ctrl+C signal*/
void trap(int signal){execute=0;}

int main(int argc, char *argv[])
{
/*Capture the Ctrl+C signal*/
signal(SIGINT,&trap);
execute=1;

int sockfd;
/* my address information */
struct sockaddr_in my_addr;
/* connector’s address information */
struct sockaddr_in their_addr;
int addr_len=0, numbytes=0;
char buf[MAXBUFLEN];


if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
{
    perror("Server-socket() sockfd error lol!");
    exit(1);
}
else
    printf("Server-socket() sockfd is OK...\n");
 
/* host byte order */
my_addr.sin_family = AF_INET;
/* short, network byte order */
my_addr.sin_port = htons(MYPORT);
/* automatically fill with my IP */
my_addr.sin_addr.s_addr = INADDR_ANY;
/* zero the rest of the struct */
memset(&(my_addr.sin_zero), '\0', 8);
 
if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
{
    perror("Server-bind() error lol!");
    exit(1);
}
else
    printf("Server-bind() is OK...\n");
 
addr_len = sizeof(struct sockaddr);

while(execute)
{
printf("\n==============\n");

/*Recieve the data*/
if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
{
    perror("Server-recvfrom() error!");
    exit(1);
}
else
{
    printf("Server-Waiting and listening...\n");
    printf("Server-recvfrom() is OK...\n");
}
 
printf("Server-Got packet from %s\n", inet_ntoa(their_addr.sin_addr));
printf("Server-Packet is %d bytes long\n", numbytes);
buf[numbytes] = '\0';
printf("Server-Packet contains \"%s\"\n", buf);

/*Print unsymmetrical or symmetrical mode*/
if(buf[0]==mode_para[0])
{
buf[3]='\0';
printf("unsymmetrical\n");
}
else
{
printf("symmetrical\n");	
}

/*send back*/
if((numbytes = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1)
{
    perror("Server-sendto() error!");
    exit(1);
}
else
    printf("Server-sendto() is OK...\n");
numbytes=0;

printf("\n==============\n");
}

/*Capture the Ctrl C signal*/
signal(SIGINT,SIG_DFL);

/*Close the port*/
if(close(sockfd) != 0)
    printf("Server-sockfd closing failed!\n");
else
    printf("Server-sockfd successfully closed!\n");

return 0;
}
