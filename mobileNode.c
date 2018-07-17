/*
	Brian Yu
	CS 6596
	Program #3
	mobileNode.c
*/
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>           /* for gettimeofday() */
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define TRUE 1
char  DELIM = '-';

/* The mobileNode gets packets from the foreignAgent. It checks to see if this is the correct
 * foreignAgent that it's connected to (simulated). If so, accept the packet. Else reject.
 * Every 5 seconds, switch foreign agents and tell the home agent the new foreign agent address.
 * Usage: mobileNode localport ha_addr ha_port fa1_addr fa1_port fa2_addr fa2_port
 */

int main(int argc,char *argv[])
{
  int sock;
  socklen_t length;
  struct sockaddr_in source, dest, haAddr, fa1Addr, fa2Addr;
  struct hostent *haHostEnt, *fa1HostEnt, *fa2HostEnt;
  char buf[1024];
  char registration[100];
  int rval,sendRval;
  int seqno;
  socklen_t srclen;
  struct timeval rcvtime;
  struct timezone zone;
  struct tm* myLocalTime;
  uint16_t sourcePort;
	long timer = 0;
	long startTime = 0;
	int runDuration =  100;
	char* currentFA;
	char* currentFAPort;
	
  /* check arguments */
  if (argc != 8) {
    printf("Usage: mobileNode localport ha_addr ha_port fa1_addr fa1_port fa2_addr fa2_port\n");
    exit(0);  
  }  

	char* FA1PortS = argv[5];
	char* FA2PortS = argv[7];

	/* create socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    perror("opening datagram socket");
    exit(1);
  }

  /* name socket using wildcard for IP address and given port number */

  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = INADDR_ANY;
  dest.sin_port = htons(atoi(argv[1]));
  if(bind(sock, (struct sockaddr *)&dest, sizeof(dest))) {
    perror("binding socket name");
    exit(1);
  }

	// home agent addr
	haAddr.sin_family = AF_INET;
	haHostEnt = gethostbyname(argv[2]);
	if(haHostEnt == 0){
		printf("%s: unknown host\n",argv[2]);
		exit(2);
	}
	bcopy(haHostEnt->h_addr, &haAddr.sin_addr, haHostEnt->h_length);
	haAddr.sin_port = htons(atoi(argv[3]));

	// foreign agent 1 addr
	fa1Addr.sin_family = AF_INET;
	fa1HostEnt = gethostbyname(argv[4]);
	if(fa1HostEnt == 0){
		printf("%s: unknown host\n",argv[4]);
		exit(2);
	}
	bcopy(fa1HostEnt->h_addr, &fa1Addr.sin_addr, fa1HostEnt->h_length);
	fa1Addr.sin_port = htons(atoi(FA1PortS));

	// foreign agent 2 addr
	fa2Addr.sin_family = AF_INET;
	fa2HostEnt = gethostbyname(argv[6]);
	if(fa2HostEnt == 0){
		printf("%s: unknown host\n",argv[6]);
		exit(2);
	}
	bcopy(fa2HostEnt->h_addr, &fa2Addr.sin_addr, fa2HostEnt->h_length);
	fa2Addr.sin_port = htons(atoi(FA2PortS));
	
	// initialize currentFA
	currentFA = inet_ntoa(fa1Addr.sin_addr);
	currentFAPort = FA1PortS;
	printf("Current Foreign Agent: %s/%s\n",currentFA, currentFAPort);
	fflush(stdout);

	
  /* find out assigned port number and print out */

  length = sizeof(dest);
  if(getsockname(sock, (struct sockaddr *)&dest, &length)) {
    perror("getting socket name");
    exit(1);
  }

	// set start time
	if (gettimeofday(&rcvtime, &zone) < 0) {
		perror("getting time");
		exit(1);
	}
	startTime = rcvtime.tv_sec;
    
  do {
    bzero(buf,sizeof(buf));
    srclen = sizeof(source);    
    rval = recvfrom(sock,buf,1024,MSG_DONTWAIT,(struct sockaddr *)&source,&srclen);
        
    // get current relative time 
    if (gettimeofday(&rcvtime, &zone) < 0) {
			perror("getting time");
			exit(1);
    }
    // check to see if 5 seconds has passed. if so, switch FA and send registration packet
    if(timer == 0)
    	timer = rcvtime.tv_sec;
    else if(timer + 5 == rcvtime.tv_sec){  // 5 seconds has elapsed
    	timer = rcvtime.tv_sec;    
    	
    	// switch FAs
    	if(strcmp(currentFA, inet_ntoa(fa1Addr.sin_addr))==0 && strcmp(currentFAPort, FA1PortS)==0){ // switch to FA2
    		currentFA = inet_ntoa(fa2Addr.sin_addr); 
    		currentFAPort = FA2PortS;
    	}else{ // switch to FA1
    		currentFA = inet_ntoa(fa1Addr.sin_addr);     	
    		currentFAPort = FA1PortS;
    	}
    	
			// send registration packet to HA
			bzero(registration, sizeof(registration) );
			registration[0] = 'r';
			registration[1] = DELIM;
			int currentFALen = strlen(currentFA);
			
			strcat(registration, currentFA);
			registration[2 + currentFALen] = DELIM;
			strcat(registration, currentFAPort);
			if ((sendRval=sendto(sock,registration,sizeof(registration),0,(struct sockaddr *)&haAddr,sizeof(haAddr))) < 0) {
				perror("writing on datagram socket");
			}
			
		  myLocalTime = localtime(&rcvtime.tv_sec);
			printf("Registration sent. Time = %d:%d:%d New care-of address = %s/%s\n", myLocalTime->tm_hour, myLocalTime->tm_min, myLocalTime->tm_sec, currentFA, currentFAPort);

    }	
    if(rval > 0){
    	// parse msg
			strtok(buf, &DELIM); // get rid of type token
		 	char* seqnoS = strtok(NULL, &DELIM);
		 	if(seqnoS == NULL){
		 		printf("seqnoS = NULL. ");
				exit(1);
			}	
			sscanf(seqnoS, "%d", &seqno);
			
			// get human readable time
		  myLocalTime = localtime(&rcvtime.tv_sec);
			
			char* sourceIP = inet_ntoa(source.sin_addr);
			sourcePort = ntohs(source.sin_port);	
			char sourcePortS[100];
			bzero(sourcePortS, sizeof(sourcePortS) );
			sprintf(sourcePortS, "%d\0", (int)sourcePort);
		  
		  printf("Sequence Number = %d Time = %d:%d:%d FA = %s/%s ", seqno, myLocalTime->tm_hour, myLocalTime->tm_min, myLocalTime->tm_sec, sourceIP,sourcePortS); 
		  // accept or reject packet
		  if(strcmp(sourceIP, currentFA) == 0 && strcmp(sourcePortS, currentFAPort)==0)
		  	printf("Accepted\n");
	  	else printf("Rejected\n");		 
  	}
  	
  	if(rcvtime.tv_sec > startTime + runDuration){
  		close(sock);
  		exit(0);  
		}
  } while (rval != 0);
  close(sock);
  return (0);  
}
