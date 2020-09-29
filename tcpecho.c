/* Should be run by the QEMUSERV */

#include "user.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

int main()
{
  unsigned char data[512];
  int s;
  int len;
  s = socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(22);
  sa.sin_addr.s_addr = inet_addr(INADDR_ANY);
  len = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &len, sizeof(int));
  bind(s, (struct sockaddr *)&sa, sizeof(sa));
  int addrlen = sizeof(sa);
//  len = lwip_recvfrom(s, data, sizeof(data), 0, &sa, &addrlen);
//  lwip_sendto(s, data, len, 0, &sa, addrlen);
  int client;
  listen(s, 1);
  while ((client = accept(s, (struct sockaddr *)&sa, &addrlen)) > 0)
  {
      do {
        len = recv(client, data, sizeof(data), 0);
//        cprintf("received %d bytes\n", len);
        send(client, data, len, 0);
        if (data[0] == '!')
            len = -1;
      } while (len > 0);

      sockclose(client);
  }
  sockclose(s);
  return 0;
}
