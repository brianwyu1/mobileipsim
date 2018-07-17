/*
	Brian Yu
	CS 6596
	Program #3
	foreignAgent.c
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

/* The foreignAgent gets packets from the homeAgent and forwards it to the mobileNode.
 * Usage: foreignAgent portnumber destination-hostname destination-portnumber
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

  /* Check arguments */
  if (argc != 4) {
    printf("Usage: foreignAgent portnumber destination-hostname destination-portnumber\n");
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

	// forward packet
	if ((rval=sendto(sock,buf,sizeof(buf),0,(struct sockaddr *)&dest,sizeof(dest))) < 0) {
		perror("writing on datagram socket");
	}
	// parse msg
	strtok(buf, &DELIM); // get rid of type token
	char* seqnoS = strtok(NULL, &DELIM);
	if(seqnoS == NULL){
		printf("seqnoS = NULL. ");
		exit(1);
	}	
	// store seqno
	sscanf(seqnoS, "%d", &seqno);

	/* get current relative time */
	if (gettimeofday(&rcvtime, &zone) < 0) {
		perror("getting time");
		exit(1);
	}
	struct tm* myLocalTime;
	myLocalTime = localtime(&rcvtime.tv_sec);

	char* destIP = inet_ntoa(dest.sin_addr);
	printf("Sequence Number = %d Time = %d:%d:%d Forwarded to %s/%d \n", seqno, myLocalTime->tm_hour, myLocalTime->tm_min, myLocalTime->tm_sec, destIP, ntohs(dest.sin_port)); 
      fflush(stdout);
  } while (rval != 0);
  close(sock);
  return (0);
}
