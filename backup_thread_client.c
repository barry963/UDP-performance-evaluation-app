/*senderprog.c - a client, datagram*/
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>


/* the port users will be connecting to */
#define MYPORT 8888
#define MAXBUFLEN 501
#define MAXRATE   1000
#define WAITWINDOWSIZE 200
#define MIN 0.0000000001

//int  WAITWINDOWSIZE=1000;

char mode_para[2]="u";
int execute;


int sockfd;
/* connector’s address information */
struct sockaddr_in their_addr;
struct hostent *he;
int numbytes,addr_len;

/*round trip variable*/
struct timeval begin[WAITWINDOWSIZE];

int waitwindowindex=0;

//struct timeval begin;
struct timeval end;
double delay,total_delay;
unsigned int send_packet_num=0,response_packet_num=0;
/*send buffer*/
unsigned char send_data[MAXBUFLEN];

/*timeout variable*/
int rc;
fd_set read_set;
struct timeval timeout;

double sending_interval;


/*recieve buffer*/
unsigned char buf[MAXBUFLEN];

/*thread*/
pthread_mutex_t mutex1=PTHREAD_MUTEX_INITIALIZER;

/*Capture the Ctrl+C signal*/
void trap(int signal)
{
execute=0;
}


/*Generate packet ID for unsigned char [2] array input*/
void GeneratePacketID(char *bin,const int index)
{
	printf("Generate Packet ID %d\n",index);
	bin[0]=index/256+1;
	bin[1]=index%256+1;
}

/*Calculate packet ID for unsigned char [2] array input*/
int CalculatePacketID(char *bin)
{
	int ID=(bin[0]-1)*256+bin[1]-1;
	printf("Calculate Packet ID %d\n",ID);
	return ID;
}

/*Check if the corresponding timeout is 0*/
int CheckTimeoutArray(const int index,struct timeval *array)
{
	printf("%d begin time %f %f\n",index,(double)array[index].tv_sec,(double)array[index].tv_usec);

	if((array[index].tv_sec==0)&&(array[index].tv_usec==0))
		return 1;
	else
		return 0;	
}

/*Check the parameter*/
int ParameterCheck(char *para)
{
	do
	{
		if((*para<48)||(*para>57))
			return 0;
	}while(*(++para)!='\0');
	return 1;
}

/*Generate the packet*/
double PacketGenrate(char *data_array,char* sending_rate,char* packet_size,char* mode)
{	
	int rate=strtol(sending_rate,NULL,10);
	if((rate<1)||(rate>MAXRATE))
	{
		printf("	Byterate error!\n\n");
		exit(1);
	}
	int size=strtol(packet_size,NULL,10);
	if((size<3)||(size>=MAXBUFLEN))
	{
		printf("	Packet size error!\n\n");
		exit(1);
	}
	char mode_type[14];

	if(!strcmp(mode,mode_para))/*unsymmetrical*/
	{
		data_array[0]=mode_para[0];	
		memcpy(mode_type,"unsymmetrical",13);
	}
	else/*symmetrical*/
	{
		memcpy(mode_type,"symmetrical",11);		
	}
	
	double time=(size+0.0)/(rate+0.0);
	data_array[size]='\0';

	printf("Client-Sending rate: %d byte/s, Packet size: %d byte, Mode type: %s\n",rate,size,mode_type);
	return time;
}


/**Compute the delay time(ms) between two time point*/
double ComputeMsDelay(struct timeval tbegin, struct timeval tend)
{
  time_t      secs  = tend.tv_sec  - tbegin.tv_sec;
  suseconds_t usecs = tend.tv_usec - tbegin.tv_usec;

  if (usecs < 0)
  {
    --secs;
    usecs += 1000000;
  }

  return ((double) secs + (double) usecs / 1000000) * 1000;
}

/*Display the UDP performance report*/
void DisplayReport(char *rate,char *size,char *mode,unsigned int total_send_num,unsigned int total_response_num,double total_delay_time)
{
	char mode_type[14];

	if(!strcmp(mode,mode_para))/*unsymmetrical*/
	{
		memcpy(mode_type,"unsymmetrical",13);
	}
	else/*symmetrical*/
	{
		memcpy(mode_type,"symmetrical",11);		
	}

	printf("\n----- UDP communication performance report -----\n");
	printf("     Client\n     Sending rate: %s byte/s\n     Packet size: %s byte\n     Mode type: %s\n",rate,size,mode_type);	
	printf("     %d packets transmitted, %d received\n",total_send_num,total_response_num);
	if(total_response_num!=0)	
	printf("     Average round trip time: %lf ms\n",total_delay_time/(total_response_num+0.0));
	else
	printf("     Average round trip time: NA ms\n");
	printf("     Packet lose ratio: %.2f\%\n\n\n",(float)(total_send_num-total_response_num+0.0)*100.0/(total_send_num+0.0));	
}

void SendData()
{
printf("\n===========\n");

/*Set the packet index num*/

//GeneratePacketID(send_data+1,waitwindowindex);

while(1)
{

	if(CheckTimeoutArray(waitwindowindex,begin))
	{
		GeneratePacketID(send_data+1,waitwindowindex);
		break;
	}
	else
	{
		begin[waitwindowindex].tv_sec=0;
		begin[waitwindowindex].tv_usec=0;
		
		if(++waitwindowindex>=WAITWINDOWSIZE)
			waitwindowindex=0;
	}
}




/*record the send start time*/
gettimeofday(&begin[waitwindowindex],0);

if(++waitwindowindex>=WAITWINDOWSIZE)
	waitwindowindex=0;

if((numbytes = sendto(sockfd, send_data, strlen(send_data), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1)
{
perror("Client-sendto() error!");
exit(1);
}
else
{
send_packet_num++;
printf("Client-sendto() is OK...\n");
printf("Client-Packet contains \"%s\"\n", send_data);
}
printf("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));


usleep(1000000*sending_interval);
	
}

void ReceiveData()
{
int select_return;
//while(1)
while((select_return=select(sockfd+1, &read_set, NULL, NULL, &timeout))>= 0)
{
	if(select_return==0)
		{
			
		}
	else
	if(FD_ISSET(sockfd, &read_set))
	{	
		/*record the receiving time*/
		gettimeofday(&end,0);
		if(((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) != -1))
		{
			//CheckTimeoutArray(int index,struct timeval *array)
			//CalculatePacketID(char *bin)
			if(CheckTimeoutArray(CalculatePacketID(buf+1),begin))
			{
				printf("Client-response timeout\n");
				continue;
			}
			else
			{
			response_packet_num++;	
	 	    	printf("Client-Waiting and listening...\n");
	     	printf("Client-recvfrom() is OK...\n");		
			}
		}	
		else
		{	
			perror("Client-recvfrom() error lol!");
 	   		exit(1);
		}


		printf("Client-Got packet from %s\n", inet_ntoa(their_addr.sin_addr));
		printf("Client-Packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("Client-Packet contains \"%s\"\n", buf);

		delay=ComputeMsDelay(begin[CalculatePacketID(buf+1)],end);

	printf("%d end time %f %f\n",CalculatePacketID(buf+1),(double)end.tv_sec,(double)end.tv_usec);

		begin[CalculatePacketID(buf+1)].tv_sec=0;
		begin[CalculatePacketID(buf+1)].tv_usec=0;

		
		total_delay+=delay;
		printf("Client-send and response delay: %f ms\n",delay);

		
	}

	break;
}

}

int main(int argc, char *argv[ ])
{
/*parameter test*/
if ((argc != 5)||(!ParameterCheck(argv[2]))||(!ParameterCheck(argv[3]))||(strcmp(argv[4],"u")&&strcmp(argv[4],"s")))
{
fprintf(stderr, "Parameter error!\nClient-Usage: %s <hostname> <sending rate (1-1000 byte/s)> <packet size (2~500 bytes)> <symmetrical or unsymmetrical (s/u)>\n", argv[0]);
exit(1);
}

/*For Ctrl C*/
signal(SIGINT,&trap);
execute=1;


/*initialize the send data*/
int i=0;
for(;i<MAXBUFLEN-1;i++)
{
	send_data[i]='0';
}
send_data[MAXBUFLEN-1]='\0';


/*initialize the timeout array*/
for(i=0;i<WAITWINDOWSIZE;i++)
{
	begin[i].tv_sec=0;
	begin[i].tv_usec=0;	
}

/*generate sending packet*/
sending_interval=PacketGenrate(send_data,argv[2],argv[3],argv[4]);


timeout.tv_sec = 1;
timeout.tv_usec = 0;


/* get the host info */
if ((he = gethostbyname(argv[1])) == NULL)
{
perror("Client-Invalid hostname gethostbyname() error!");
exit(1);
}
else
printf("Client-gethostname() is OK...\n");

 
if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
{
perror("Client-socket() error lol!");
exit(1);
}
else
printf("Client-socket() sockfd is OK...\n");
 
/* host byte order */
their_addr.sin_family = AF_INET;
/* short, network byte order */
printf("Using port: 8888\n");
their_addr.sin_port = htons(MYPORT);
their_addr.sin_addr = *((struct in_addr *)he->h_addr);
/* zero the rest of the struct */
memset(&(their_addr.sin_zero), '\0', 8);

/*Thread initialize*/
pthread_t sendthread,receivethread;

if(pthread_create(&sendthread,NULL,&SendData,NULL))
{
	printf("Client-Thread SendData creation failed\n");
	exit(1);
}

if(pthread_create(&receivethread,NULL,&ReceiveData,NULL))
{
	printf("Client-Thread ReceiveData creation failed\n");
	exit(1);
}

pthread_join(sendthread,NULL);
pthread_join(receivethread,NULL);


/*send and response analysis process*/
while(execute)
{

//send
SendData();

/*receive data back from the server*/
addr_len = sizeof(struct sockaddr);

FD_ZERO(&read_set);
FD_SET(sockfd, &read_set);


//receive

ReceiveData();

/*
if(select(sockfd+1, &read_set, NULL, NULL, &timeout) < 0)//timeout
{
	printf("Client-response timeout\n");
}
*/




printf("\n===========\n");

}

signal(SIGINT,SIG_DFL);

if (close(sockfd) != 0)
printf("Client-sockfd closing is failed!\n");
else
{
	printf("Client-sockfd successfully closed!\n\n");
	DisplayReport(argv[2],argv[3],argv[4],send_packet_num,response_packet_num,total_delay);
}
return 0;

}
