// xv6Chat server. 
//
// A simple IRC server implementation.
//
// To be run within host linux. Version 16.04 preferred.
// 
// Compilation: "make server"
// Usage: "./xv6ChatServer <Server IP> <Server Port>"
//


/** xv6 support is no longer supported. Run Server on host machine instead. **/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define SNDBUFSIZE 256 //64
#define RCVBUFSIZE 256 //64

// #define VERBOSE

int masterSocket;

void
cleanup(int sig)
{
	close(masterSocket);
	printf("xv6Chat Server terminating, have a nice day :P\n");
	exit(1);
}

int 
main(int argc, char *argv[]) 
{
	signal(SIGINT, cleanup);

	// Global Vars
	char* ip;
	unsigned short port;
	int clientSocket[4];
	struct sockaddr_in servAddr;
	unsigned int uservAddrLen;	
	fd_set fds;
	int sdMax;
	int i;
	int newClientSocket;
	int msgSizeRcv;
	char rcvBuf[RCVBUFSIZE];
	char sndBuf[SNDBUFSIZE];
	int readNum;
	int j;
	char exitMsg[6];
		
	// Variable initalisation
	struct user_info{
		char uname[32];
	};

	struct user_info client0 = {""};
	struct user_info client1 = {""};
	struct user_info client2 = {""};

	// Message from client to signify exit == "\exit"
	exitMsg[0] = 92;
	exitMsg[1] = 101;
	exitMsg[2] = 120;
	exitMsg[3] = 105;
	exitMsg[4] = 116;
	exitMsg[5] = 0;

	
	if(argc != 3){
		printf("Incorrect number of arguments! Correct format is\n\t./xv6ChatServer <Server IP> <Server Port>\n");
		exit(1);
	}
	if((strlen(argv[1]) < 7) || (atoi(argv[2]) > 65535)){
		// Passed in IP is length less than 1.1.1.1. Not a valid Ip addr (crude check)
		// Port is larger than largest unsigned 16-bit number. Not valid port
		printf("Passed in arguments are not valid! Correct format is\n\t./xv6ChatServer <Server IP> <Server Port>\n");
		exit(1);
	}
	ip = argv[1];
	port = atoi(argv[2]);	
	

	// Create a new TCP master socket
	if((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server master socket establishment error!\n");
		exit(1);
	}
	
	#ifdef VERBOSE
	printf("[Server] Socket created\n");
	#endif
	
	// Initalise client socket array
	for(i = 0; i < 4; i++){
		clientSocket[i] = 0;
	}
	
	#ifdef VERBOSE
	printf("[Server] Client Socket Array created\n");
	#endif
	
	// Initalise address structure
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	if(inet_pton(AF_INET, ip, &servAddr.sin_addr) <= 0){
		printf("Invalid Server IP\n");
		exit(1);
	}
	
	#ifdef VERBOSE
	printf("[Server] Address structure created\n");
	#endif
	
	// Bind master socket
	if(bind(masterSocket, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
    	printf("Server bind error!\n");
    	exit(1);
    }
	
	#ifdef VERBOSE
	printf("[Server] Master socket bound\n");
	#endif
	
	// Listen on master socket
  if(listen(masterSocket, 4) < 0){
    	printf("Server listen error!\n");
    	exit(1);
    } 
  
  #ifdef VERBOSE
  printf("[Server] Waiting for client activity...\n");
  #endif
	
	uservAddrLen = sizeof(servAddr);

	printf("xv6Chat Server start..\n");				

	// Server Operation Logic
	while(1)
	{
		// Clear FD set
		FD_ZERO(&fds);
		
		// Add master socket
		FD_SET(masterSocket, &fds);
		sdMax = masterSocket;
	
		// Add client sockets to FD set
		for(i = 0; i < 4; i++){		
		
			// Add to FD set
			if(clientSocket[i] > 0){
				FD_SET(clientSocket[i], &fds);	
			}
			
			// Update the largest sd
			if(clientSocket[i] > sdMax){
				sdMax = clientSocket[i];
			}
		}
	
		// Call select and check for errors
		if(select(sdMax+1, &fds, 0, 0, 0) < 0){
			printf("Select error!\n");
			exit(1);
		}

		// Accept and handle incoming connection
		if(FD_ISSET(masterSocket, &fds)){
			if((newClientSocket = accept(masterSocket, (struct sockaddr*)&servAddr, &uservAddrLen)) < 0){
				printf("New client accept error!\n");
				exit(1);
			}
			
			// Add inital connection to array of clients
			for(i = 0; i < 4; i++){
				
				// Reject 4th connection
				if(i == 3){
					clientSocket[i] = newClientSocket;
				
					// Receive inital empty mesage from client
					memset(rcvBuf, 0, sizeof(rcvBuf));
					if(recv(clientSocket[i], rcvBuf, RCVBUFSIZE, 0) < 0){
						printf("Message receive from new client error!\n");
						exit(1);
					}
				
					// Send overload message
					memset(sndBuf, 0, sizeof(sndBuf));
					strcpy(sndBuf, "Closing 4th connection");
					
					if(send(clientSocket[i], sndBuf, SNDBUFSIZE, 0) < 0){
  					printf("Server send error!\n");
  					exit(1);
  				}
  				
  				// close socket connection
					close(clientSocket[i]);
					clientSocket[i] = 0;
					break;
				}	
				
				
				if(clientSocket[i] == 0){
					clientSocket[i] = newClientSocket;;
					
					// Receive inital username mesage from client
					memset(rcvBuf, 0, sizeof(rcvBuf));
					if((msgSizeRcv = recv(clientSocket[i], rcvBuf, RCVBUFSIZE, 0)) < 0){
						printf("Message receive from new client error!\n");
						exit(1);
					} 

					
					// Remove newline character
					if(rcvBuf[strlen(rcvBuf)-1] == 10){
						rcvBuf[strlen(rcvBuf)-1] = '\0';
					}
					
					#ifdef VERBOSE
					printf("[Server] Message received: %s\n", rcvBuf);
					#endif
					
					if(i == 0){
						strcpy(client0.uname, rcvBuf);
					}else if(i == 1){
						strcpy(client1.uname, rcvBuf);
					}else if(i == 2){
						strcpy(client2.uname, rcvBuf);
					}
				
					printf("User {%s} connected.\n", rcvBuf);
				
					// Packetize broadcast msg
					memset(sndBuf, 0, sizeof(sndBuf));
					strcat(sndBuf, "[Server] User: {");
					strcat(sndBuf, rcvBuf);
					strcat(sndBuf, "} connected. Everyone say hello! :)");
					
					#ifdef VERBOSE
					printf("[Server] Message sending: %s\n", sndBuf);
					#endif
					
					// Send msg to all connected clients
					for(j = 0; j < 4; j++){
						if((clientSocket[j] != 0)){
							if(send(clientSocket[j], sndBuf, SNDBUFSIZE, 0) < 0){
	  							printf("Server send error!\n");
	  							exit(1);
  							}
						}						
					}
					break;
				}
			}
		}
			
				
		// Update connection on other sockets
		for(i = 0; i < 4; i++){
			
			if(FD_ISSET(clientSocket[i], &fds)){
													
					// Check for recv error and close socket if zombie client
					memset(rcvBuf, 0, sizeof(rcvBuf));
					
					if((readNum = recv(clientSocket[i], rcvBuf, RCVBUFSIZE, 0)) < 0){
						printf("Message receive from client error!\n");
						exit(1);
					}else if(readNum == 0){
						close(clientSocket[i]);
						clientSocket[i] = 0;
					}
					
					if(rcvBuf[0] == 0){
						// poll upate. Don't do anything with this packet
						break;
					}
						
					if(strcmp(rcvBuf, exitMsg) == 0){
						// Client sent exit message
						#ifdef VERBOSE
						printf("[Server] Closing client %d\n", i);
						#endif
						close(clientSocket[i]);
						clientSocket[i] = 0;
						
						// Notify other clients of client's departure
						memset(sndBuf, 0, sizeof(sndBuf));
						char disconnectName[32];
						memset(disconnectName, 0, sizeof(disconnectName));
						
						strcat(sndBuf, "[Server] User: {");
						if(i == 0){
							strcat(disconnectName, client0.uname);
						}else if(i == 1){
							strcat(disconnectName, client1.uname);
						}else if(i == 2){
							strcat(disconnectName, client2.uname);
						}
						strcat(sndBuf, disconnectName);
						strcat(sndBuf, "} has disconnected.");
						
						i = 4; //To skip msg receive

						// Notify server monitors
						printf("User {%s} disconnected.\n", disconnectName);
						goto servsend;
					}
						
					if(i == 0){
						// client0
						#ifdef VERBOSE
						printf("[Server] Message received: %s From client: %d\n", rcvBuf, i);
						#endif
						
						// Packetize message
						memset(sndBuf, 0, sizeof(sndBuf));
						strcat(sndBuf, "{");
						strcat(sndBuf, client0.uname);
						strcat(sndBuf, "} ");
						strcat(sndBuf, rcvBuf);
							
					}else if(i == 1){
						// client 1
						#ifdef VERBOSE
						printf("[Server] Message received: %s From client: %d\n", rcvBuf, i);
						#endif
						
						memset(sndBuf, 0, sizeof(sndBuf));
						strcat(sndBuf, "{");
						strcat(sndBuf, client1.uname);
						strcat(sndBuf, "} ");
						strcat(sndBuf, rcvBuf);
						
				}else if(i == 2){
						// client 2
						#ifdef VERBOSE
						printf("[Server] Message received: %s From client: %d\n", rcvBuf, i);
						#endif
						
						memset(sndBuf, 0, sizeof(sndBuf));
						strcat(sndBuf, "{");
						strcat(sndBuf, client2.uname);
						strcat(sndBuf, "} ");
						strcat(sndBuf, rcvBuf);
						
					}
					
servsend:					
					// Send message to all connected clients
					for(j = 0; j < 4; j++){
					
						if((clientSocket[j] != 0)){	
							int successSend;
							if((successSend = send(clientSocket[j], sndBuf, SNDBUFSIZE, 0)) < 0){
  							printf("Server send error!\n");
  							exit(1);
  						}else if(successSend == 0){
  							//Client disconnected
  							close(clientSocket[i]);
  							clientSocket[i] = 0;
  						}
						}						
					}
					
				
			}
		}
		

	} 

	// exit(1);
	cleanup(SIGINT);
	return 0;
}
