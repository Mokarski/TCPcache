/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include "network.h"
#include "signals.h"
#include "speedtest.h"
#include <errno.h>





int main(int argc , char *argv[])
{

//INIT SIGNALS     
    printf("MAX_Signals [%i] \n",MAX_Signals);
    init_signals_list(); // erase signal lsit 
socket_init();
while (1){

	    speedtest_start(); //time start

	    //======================== read all 485 signals from server create signals and virtual devices ===================

	    if ( tcpsignal_read(".") == 0 ){ // if we get response from server, get all signals list
                 tcpsignal_parser(signal_parser_buf);
                 printf("Recived Buffer[%s] \n\r",signal_parser_buf);
               }
               
      //    socket_close();
    
              printf("=================== ==>   SPEEDTEST Time load signals: [ %ld ] ms. \n\r", speedtest_stop());     
  
  
	       speedtest_start(); //time start
	       int z=0;

	       for (z=0; z < MAX_Signals; z++) {

	            if ( sDeSerial_by_num (z) == 0){
            
    		        printf ("Name:[%s] Val:[%i]",Signal_Array[z].Name,Signal_Array[z].Value[1]); //debug
	                //print_by_name(Signal_Array[z].Name);
	                if (Signal_Array[z].Value[1] > 0)
                        printf ("Name:[%s] Val:[%i]",Signal_Array[z].Name,Signal_Array[z].Value[1]);
                
	                } else break;
    		   }
    		   


		  printf(" ==>   SPEEDTEST Deserial signals signals: [ %ld ] ms. \n\r", speedtest_stop());     

       }
 socket_close();


return 0;
}
