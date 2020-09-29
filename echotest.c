#include "user.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#define BUFFSIZE 32
#define IPADDR "10.0.3.17"
#define PORT 7

char *msg = "Hello world!\n";

static void
die(char *m)
{
	printf(2, "%s\n", m);
	exit();
}

int 
main(int argc, char **argv)
{
	int sock;
	struct sockaddr_in echoserver;
	char buffer[BUFFSIZE];
	unsigned int echolen;
	int received = 0;

	// if (argc != 2)
	// 	die("Must give IP address as arg\n");
	char *IPADDR = argc >= 2 ? argv[1] : "127.0.0.1";

	printf(1, "Connecting to:\n");
	printf(1, "\tip address %s = %x\n", IPADDR, inet_addr(IPADDR));

	// Create the TCP socket
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die("Failed to create socket");

	printf(1, "opened socket\n");

	// Construct the server sockaddr_in structure
	memset(&echoserver, 0, sizeof(echoserver));       // Clear struct
	echoserver.sin_family = AF_INET;                  // Internet/IP
	echoserver.sin_addr.s_addr = inet_addr(IPADDR);   // IP address
	echoserver.sin_port = htons(PORT);		  // server port

	printf(1, "trying to connect to server\n");
	connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver));
	// Establish connection
	if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
		die("Failed to connect with server");

	printf(1, "connected to server\n");

	// Send the word to the server
	echolen = strlen(msg);
	if (write(sock, msg, echolen) != echolen)
		die("Mismatch in number of sent bytes");

	// Receive the word back from the server
	printf(1, "Received: \n");
	while (received < echolen) {
		int bytes = 0;
		if ((bytes = read(sock, buffer, BUFFSIZE-1)) < 1) {
			die("Failed to receive bytes from server");
		}
		received += bytes;
		buffer[bytes] = '\0';        // Assure null terminated string
		printf(1, buffer);
	}
	printf(1, "\n");

	close(sock);
}
