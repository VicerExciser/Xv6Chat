// xv6Chat client. 
//
// A simple IRC client implementation.
//
// To be run within xv6.
// 
// Compilation: "make qemu-nox"
// Usage: "xv6Chat <Server IP> <Server Port>"
//

#include "stdio.h"
// #include "nettypes.h"
#include "user.h" 		// <-- user.h now includes lwip/nettypes.h

#define SNDBUFSIZE 256 
#define RCVBUFSIZE 256 
#define UNAMESIZE  32


static char*
strncpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

char rcvBuf[RCVBUFSIZE];
char sndBuf[SNDBUFSIZE];
char uin[SNDBUFSIZE]; //[UNAMESIZE];
char blank[SNDBUFSIZE];

int main(int argc, char *argv[]) {

	// Global Vars
	char* ip;
	unsigned short port;
	int clientSocket;
	struct sockaddr_in servAddr;
	int i;
	int a;
	int time;
	

	if(argc != 3){
		printf(1, "Incorrect number of arguments! Correct format is\n\txv6Chat <Server IP> <Server Port>\n");
		exit();
	}
	if((strlen(argv[1]) < 7) || (atoi(argv[2]) > 65535)){
		// Passed in IP is length less than 1.1.1.1. Not a valid Ip addr (crude check)
		// Port is larger than largest unsigned 16-bit number. Not valid port
		printf(1, "Passed in arguments are not valid! Correct format is\n\txv6Chat <Server IP> <Server Port>\n");
		exit();
	}
	ip = argv[1];
	port = atoi(argv[2]);
			
	
	// Variable and Buffer init
	memset(sndBuf, 0, sizeof(sndBuf));
	memset(rcvBuf, 0, sizeof(rcvBuf));
	time = 0;
	memset(blank, 0, sizeof(blank));
	
	
	/** TCP Connection **/
	
	// Create a new socket
  if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
  	printf(1, "Client socket establishment error!\n");
   	exit();
  }
  
  
  // Initalise address structure
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(port); 
  servAddr.sin_addr.s_addr = inet_addr(ip);

	
	// Establish server connection
	if(connect(clientSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0){
   	printf(1, "Client connection error!\n");
   	exit() ;
  }
  

	/** We are connected to the server now **/
  
  // Getting client's username
 	memset(uin, 0, sizeof(uin));
 	printf(1, "Please enter your username: (Max 30 chars)\n");
 	gets(uin, UNAMESIZE-1);
 	strncpy(sndBuf, uin, UNAMESIZE);
	
  
  // Sending username packet to server
  if(send(clientSocket, sndBuf, SNDBUFSIZE, 0) < 0){
  	printf(1, "Client send error!\n");
  	exit();
  }
  memset(sndBuf, 0, sizeof(sndBuf));
  
  // Printing initial packet
  int bytesrcvd = 0;
	if((bytesrcvd = recv(clientSocket, rcvBuf, RCVBUFSIZE, 0)) < 0){
		printf(1, "Message receive from new server error!\n");
		exit();
	}
	
	printf(1, "--- Welcome to xv6Chat. Play nice :) ---\n");
	rcvBuf[bytesrcvd] = '\0';
	printf(1, "%s\n", rcvBuf);
	
	
	memset(uin, 0, sizeof(uin));
	memset(rcvBuf, 0, sizeof(rcvBuf));
	
	// Sending loop
  while(1)
  {
  	time++;
	
	// Wait for user input, switching from recv and getting characters (inneficient as heck)

	// Getting client's message	
	for(i = 0; i < 500; i++){
		a = getc();
		if(a != 0){
			// User hit a key
			break;
		}
	}
	
	if(a != 0){
		// Add a to user input
		for(i = 0; i < sizeof(uin); i++){
			if(uin[i] == 0){
				uin[i] = (char)a;
				break;
			}
		}
	}
	
	// User hit enter
	if(a == 10){
		if(i < sizeof(uin)-1){
			uin[i] = '\0';
		}else{
			uin[sizeof(uin)-1] = '\0';
		}
	
		strncpy(sndBuf, uin, strlen(uin));

	  // Sending msg packet to server
  	// if(send(clientSocket, sndBuf, SNDBUFSIZE, 0) < 0){
  	if(send(clientSocket, sndBuf, strlen(sndBuf), 0) < 0){
  		printf(1, "Client send error!\n");
  		exit();
  	}

  	memset(uin, 0, sizeof(uin));
  	memset(sndBuf, 0, sizeof(sndBuf));
  	a = 0;

	}else if((time % 500) == 0){
		// Poll the server to prevent TCP deadlock. It's weird, but it works.
  	if(send(clientSocket, blank, SNDBUFSIZE, 0) < 0){
  		printf(1, "Client poll send error!\n");
  		exit();
  	}
  	time = 0;
  }
	
	int ok;
	memset(rcvBuf, 0, sizeof(rcvBuf));
	if((ok = recv(clientSocket, rcvBuf, RCVBUFSIZE, MSG_DONTWAIT)) > 0){
		// There is a new message
		rcvBuf[ok] = '\0';
		printf(1, "%s\n", rcvBuf);
	}else if(ok == 0){
		  printf(1, "Connection closed. Thank you for chatting! :)\n");
			exit();
	}
	
	
	}
	
	
	exit();
  return 0;
}
