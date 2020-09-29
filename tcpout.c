/* Should be run by the QEMUCLIENT */

#include "user.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"


int 
main(int argc, char *argv[])
{
  unsigned char data[512];
  int s;
  int len;
  
  s = socket(PF_INET, SOCK_STREAM, 0);
  
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(80);
  sa.sin_addr.s_addr = inet_addr("10.0.3.15");
  len = 1;
  
  setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &len, sizeof(int));
    connect(s, (struct sockaddr *)&sa, sizeof(sa));
  
  char* msg = "Hello World!";
  send(s, msg, sizeof(msg), 0);

  sockclose(s);
  exit();
}
