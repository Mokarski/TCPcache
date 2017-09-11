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
#include <unistd.h>


int main(int argc , char *argv[])
{

//*************INIT SIGNALS *********************    
    printf("MAX_Signals [%i] \n",MAX_Signals);
    init_signals_list(); // erase signal lsit 

//************* CREATE SOCKET *********************
//if (socket_init("127.0.0.1") !=0){
//     printf ("No Connection to server\n\r");
//     return; //return 0 if all OK else return 1
//     }
     
     
     if (socket_init2() !=0){
     printf ("No Connection to server\n\r");
     return; //return 0 if all OK else return 1
     }
     
     //ok
     /*
               int u=0;
                    for (u=0; u < MAX_Signals; u++) {
	            printf("1Signal preparse: Name{%s} Val0[%i]  Val1[%i] \n\r",Signal_Array[u].Name,Signal_Array[u].Value[0]  ,Signal_Array[u].Value[1]); //-48
                    }
                    
     return 0;
     */
     
//******************* WORK CYCLE *******************
int delay = 0; //1 cycle 1 ms
int tcpresult=0;
while (1){
	    printf("THIS IS LoGiC \n\r");

	    speedtest_start(); //time start
	    
	    //frame_read_s(".",1);
	    
//	    char tst[MAX_MESS];
	    
//	    frame_pack("rd","485.kb.kei1.mode1",message);
//	    frame_pack("wr","485.kb.kei1.mode1",message);
//	    printf("Messasge: [%s]\n\r",message);
	    
//	    frame_unpack (message,tst);


	    //======================== read all 485 signals from server create signals and virtual devices ===================
	    strcpy(message,""); //erase buffer
	    printf("Buffer befor create Messasge: [%s] \n\r",message);
	    frame_pack("rd", ".", message);
	    printf("Message: [%s] \n\r",message);	    
	    
		//tcpresult = frame_tcpreq(message);                      //send and recive response from server and copy to global signal_parser_buf
		
		tcpresult = frame_tcpreq(message); 
		
		printf ("tcp send result[%i]\n\r",tcpresult);	    
		
		Data_to_sName (signal_parser_buf);     	            // explode signals by delimiter ";"	    
		
	        printf("Recived Buffer ==== [%s] \n\r",signal_parser_buf); //-48	     
                strcpy(signal_parser_buf,""); 			//erase buffer before next iteration
                printf("=================== ==>   SPEEDTEST Time load signals: [ %ld ] ms. \n\r", speedtest_stop());         
  	        
  	        speedtest_start(); //time start //deserial test  	         
  	        
	        int z=0;		
	        for (z=0; z < MAX_Signals; z++) {
	            //printf(" |Signal preparse: Name{%s} Val0[%i]  Val1[%i]| \n\r",Signal_Array[z].Name, Signal_Array[z].Value[0] , Signal_Array[z].Value[1]); //48
	            int test=0;
	            //test =  sDeSerial_by_num (z);
	            char buffer[350]="";
	            strcpy (buffer, Signal_Array[z].Name);
	            test = unpack_signal(buffer  ,z); //from buffer to signal with number Z
	            
	            if (Signal_Array[z].Value[1] > 0) {
	                printf (" Name:[%s] Val:[%i] Ex[%i] \n\r",Signal_Array[z].Name,Signal_Array[z].Value[1],Signal_Array[z].ExState);	                    
	                }

    		    }
                 
		  printf(" ==>   SPEEDTEST Deserial signals signals: [ %ld ] ms. \n\r", speedtest_stop());     

         
        printf("end iteration.. \n\r");     
       }

//*************CLOSE SOCKET***************
 socket_close();


return 0;
}
