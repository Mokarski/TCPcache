/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include "/home/opc/Kombain/test/include/modbus/modbus.h"
#include "network.h"
#include "signals.h"
#include "virtmb.h"
#include "speedtest.h"


int main(int argc , char *argv[])
{

//INIT SIGNALS     
    printf("MAX_Signals [%i] \n",MAX_Signals);
    init_signals_list(); // erase signal lsit 
    
while (1){

speedtest_start(); //time start
//  write all 485 signals
    socket_init();
    tcpsignal_write("485",3);
    socket_close();

//======================== read all 485 signals from server create signals and virtual devices ===================
    socket_init();
    if ( tcpsignal_read("485") == 0 ){ // if we get response from server
        tcpsignal_parser(signal_parser_buf);
    }
    socket_close();
    

    int z=0;

       for (z=0; z < MAX_Signals; z++) {

            if ( sDeSerial_by_num (z) == 0){
//                print_by_name(Signal_Array[z].Name);
                virt_mb_filldev(Signal_Array[z].Name, Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num); //init device list
             } else break;
        }
     //virt_mb_devlist(); //show virtdev list
  printf(" ==>   SPEEDTEST Time: [ %ld ] ms. \n\r", speedtest_stop());     



}


return 0;
}
