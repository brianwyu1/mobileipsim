/*
	Brian Yu
	CS 6596
	Program #3
	dataSource.c
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
#include <inttypes.h>

/* This is the data source program. It sends 1 packet every second. Set the total number of packets 
 * with runDuration.  Specify the hostname and port number at the command line.
 * The form of the command line is `dataSource hostname portnumber'. 
 * The hostname might be algebra.sci.csueastbay.edu.  The port number
 * must be between 1000 and 64000 and must be the same as the
 * port number the receiver is running on.
 */

int main(int argc,char *argv[])
{
  int sock;
  struct sockaddr_in source, dest;
  struct hostent *hp, *gethostbyname();
  char buf[1024];  // if buf[0] = 'd', it's data. if it's 'r', it's registration
  char intToS[4];
  int seqno, rval;
  int runDuration = 100;
  struct timezone zone;

  /* Check arguments */
  if (argc != 3) {
    printf("Usage: dataSource hostname portnumber\n");
    exit(0);
  }

	/* create socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    perror("Opening stream socket");
    exit(1);
  }
  
  /* now bind source address. Port is wildcard, doesn't matter. */
  source.sin_family = AF_INET;
  source.sin_addr.s_addr = INADDR_ANY;
  source.sin_port = 0;
  if (bind(sock, (struct sockaddr *)&source, sizeof(source)) < 0) {
    perror("binding socket name");
    exit(1);
  }
	
	// set destination
  dest.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  if(hp == 0) {
    printf("%s: unknown host\n",argv[1]);
    exit(2);
  }

  bcopy(hp->h_addr, &dest.sin_addr, hp->h_length);
  dest.sin_port = htons(atoi(argv[2]));

	int port = strtoimax(argv[2], NULL, 10);
  struct timeval myTimeVal;	
  struct tm* myGmTime;

  /* create a packet */
  for(seqno = 1; seqno <= runDuration ; seqno++){  
    sprintf(intToS, "%d\0", seqno);
		buf[0] = 'd'; // d for data
		buf[1] = '-';
		buf[2] = '\0';
		strcat(buf, intToS);
	  if ((rval=sendto(sock,buf,sizeof(buf),0,(struct sockaddr *)&dest,sizeof(dest))) < 0) {
			perror("writing on datagram socket");
	  }
	  
	  if (gettimeofday(&myTimeVal, &zone) < 0) {
    	perror("getting time");
    	exit(1);
		}
    
	  myGmTime = localtime(&myTimeVal.tv_sec);
	  
    char* destIP = inet_ntoa(dest.sin_addr);	
	  printf("Sequence Number = %d Time = %d:%d:%d Dest = %s/%d\n", seqno, myGmTime->tm_hour, myGmTime->tm_min, myGmTime->tm_sec, destIP, port);
   	fflush(stdout);
   	
  	sleep(1);	  
	}
  close(sock);
  return (0);
}
