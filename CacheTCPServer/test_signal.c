#include <stdio.h>         //printf
#include <stdlib.h>
#include <string.h>        //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h>     //inet_addr
#include "signals.h"      //signasl

int main(int argc , char *argv[])
{
 init_signals_list(); 
 signals_file_load();   
 printf("Signals:\n\n\r");
 print_signals_list(); //fill signals and echo to screen
     
     
return 0;
}