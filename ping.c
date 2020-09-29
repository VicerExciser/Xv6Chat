// C program to Implement Ping 
#include "user.h"
  
int 
main(int argc, char *argv[]) 
{   
    if (argc != 2)
        printf(2, "Format Error:  ping <IP Address>\n"); 
    else if (send_ping(argv[1]) < 0)
        printf(2, "PING ERROR\n");
    else {
        while ((void*)find_proc("ping thread") != 0)
            sleep(10);
        printf(1, "--- %s ping finished ---\n", argv[1]);
    }
    exit();
} 