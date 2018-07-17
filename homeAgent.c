/*
	Brian Yu
	CS 6596
	Program #3
	homeAgent.c
*/


#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>		
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* The homeAgent program receives data packets from the dataSource 
 * and registration packets from the mobilenode.
 * It forwards the data packets to the foreignAgent stored in careOfAddr/careOfPort 
 * It updates the careOfAddr/careOfPort with the data from the registration packets.
 * The form of the command line is `hostAgent hostAgent-portnumber destination-hostname destination-portnumber'. 
 */
char  DELIM = '-';

int main(int argc,char *argv[])
{
  int sock;
  struct sockaddr_in source, middleSocket, dest;
  struct hostent *hp, *gethostbyname();
  char buf[1024];
  int seqno, rval;
  socklen_t srclen;
  struct timeval rcvtime;
  struct timezone zone;
	struct tm* myLocalTime;
  /* Check arguments */
  if (argc != 4) {
    printf("Usage: hostAgent hostAgent-portnumber destination-hostname destination-portnumber\n");
    exit(0);
  }

/* create socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    perror("Opening stream socket");
    exit(1);
  }
  
  /* now bind middleSocket address. Port is from commandline. */
  middleSocket.sin_family = AF_INET;
  middleSocket.sin_addr.s_addr = INADDR_ANY;
  middleSocket.sin_port = htons(atoi(argv[1]));

  if (bind(sock, (struct sockaddr *)&middleSocket, sizeof(middleSocket)) < 0) {
    perror("binding socket name");
    exit(1);
  }

/* initialize destination address values using commandline args*/
  dest.sin_family = AF_INET;
  hp = gethostbyname(argv[2]);
  if(hp == 0) {
    printf("%s: unknown host\n",argv[2]);
    exit(2);
  }

  bcopy(hp->h_addr, &dest.sin_addr, hp->h_length);
  dest.sin_port = htons(atoi(argv[3]));

/* receive input */
 do {		
	bzero(buf,sizeof(buf));
	srclen = sizeof(source);
	while ((rval = recvfrom(sock,buf,1024,0,(struct sockaddr *)&source,&srclen))<0){
		perror("receiver recvfrom");
	}
	char type[2];
	type[0] = buf[0];
	type[1] = '\0';
	
	if(strcmp(type,"d") == 0){ // for data packets, forward it and print out a message
		// forward packet
		if ((rval=sendto(sock,buf,sizeof(buf),0,(struct sockaddr *)&dest,sizeof(dest))) < 0) {
			perror("writing on datagram socket");
		}
		
		strtok(buf, &DELIM); // get rid of type. can't do this before forwarding packet else msg gets messed up	

		// store seqno
		char* seqnoS = strtok(NULL, &DELIM);
		if(seqnoS == NULL){
			printf("seqnoS = NULL. ");
			exit(1);
		}	
		sscanf(seqnoS, "%d", &seqno);
		
		/* get current relative time */
		if (gettimeofday(&rcvtime, &zone) < 0) {
			perror("getting time");
			exit(1);
		}
		myLocalTime = localtime(&rcvtime.tv_sec);
	
		char* destIP = inet_ntoa(dest.sin_addr);
		printf("Sequence Number = %d Time = %d:%d:%d Forwarded to %s/%d \n", seqno, myLocalTime->tm_hour, myLocalTime->tm_min, myLocalTime->tm_sec, destIP, ntohs(dest.sin_port)); 
      fflush(stdout);
	}
	
	else if(strcmp(type,"r") == 0){ // for registration packets, update CoA & print message
		strtok(buf, &DELIM);	// get rid of type. can't do this before forwarding packet else msg gets messed up
		char* careOfAddr = strtok(NULL, &DELIM);
		char* careOfPort = strtok(NULL, &DELIM);
		
		// update CoA
		hp = gethostbyname(careOfAddr);
	  if(hp == 0) {
			printf("%s: unknown host\n",careOfAddr);
			exit(2);
	  }

	  bcopy(hp->h_addr, &dest.sin_addr, hp->h_length);
	  dest.sin_port = htons(atoi(careOfPort));
	  
		/* get current relative time */
		if (gettimeofday(&rcvtime, &zone) < 0) {
			perror("getting time");
			exit(1);
		}
		myLocalTime = localtime(&rcvtime.tv_sec);
		
		printf("Registration packet received. Time = %d:%d:%d Changing care-of address to %s/%s\n", myLocalTime->tm_hour, myLocalTime->tm_min, myLocalTime->tm_sec, careOfAddr,careOfPort );
	}
	
  } while (rval != 0);
  close(sock);
  return (0);
}
