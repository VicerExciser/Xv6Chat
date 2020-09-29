#include "types.h"
#include "user.h"

int main(void) 

{
  int MAC_SIZE = 18;
  // char* ip = "192.168.2.1";
  // char* ip = "10.0.2.2";
  // char* ip = "10.0.2.15";
  char* ip = "10.0.3.17";
  char* mac = malloc(MAC_SIZE);
  if (arp("mynet0", ip, mac, MAC_SIZE) < 0) {
    printf(1, "ARP for IP:%s Failed.\n", ip);
  }
  printf(1, "IP %s has mac %s\n", ip, mac);
  if (strcmp(mac, "52:54:00:12:35:02") == 0)
  	printf(1, "Your ARP request has reached VirtualBox's internal network controller!\n");
  exit();
}
