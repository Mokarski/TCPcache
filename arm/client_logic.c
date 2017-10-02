/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>

#define  DEBUG 1
#include "network.h"
#include "signals.h"
#include "speedtest.h"
#include <errno.h>
#include <unistd.h>
#include "hash.h"
#define  DEBUG 1

//char SignalHash[MAX_Signals][MAX_Signals]; //array for store index of Signals

int Set_Signal_Param (int Signal_Array_id, char *SearchedName, int Ex ,int val){

    if ( strstr(Signal_Array[Signal_Array_id].Name,SearchedName) != NULL)
       {
        Signal_Array[Signal_Array_id].ExState = Ex;
        Signal_Array[Signal_Array_id].Value[1] = val; 
        return 1;
       }
return  0;
}

int FillSignalIndex(void){ //fill the id of signals in cyrrent signals list;

  int n=0;
  for (n=0; n < MAX_Signals; n++){
  Signal_Array[n].Srv_id_num = n;  
  }
return 0;
}

int HashTable[4000]; //key table HASH = iindex of array and content = id signals
int FillHash(void){ //fill the id of signals in cyrrent signals list;

  int n=0;
  int h;
  for (n=0; n < MAX_Signals; n++){
  
  h=Hash_id(Signal_Array[n].Name );
  
  HashTable[h]=n;
  printf("NAME[%s] HASH[%i] id[%i] \n\r",Signal_Array[n].Name ,h,n);
  }
return 0;
}

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
char tst[MAX_MESS];
char packed_txt_string[MAX_MESS];
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
		//in this place need to unpack signals from frame
		if ( frame_unpack(signal_parser_buf,tst) < 0){
		    printf ("ERROR UNPACK! \n\r");
		     printf(">>>>RECIVED^{%s} \n\r",tst);
		    break;
		    }
		if ( DEBUG == 1 ) printf(">>>>RECIVED^{%s} \n\r",tst);
		//printf ("\n\r FRAME_UNPACK: \n\r %s\n\r",tst);
		//Data_to_sName (signal_parser_buf);     	            // explode signals by delimiter ";"	  and copy to Signal.Name[]
		
		Data_to_sName (tst);
		
	        //printf("Recived Buffer ==== [%s] \n\r",signal_parser_buf); //-48	     
                strcpy(signal_parser_buf,""); 			//erase buffer before next iteration
                
                strcpy(tst,"");
                printf("=================== ==>   SPEEDTEST Time load signals: [ %ld ] ms. \n\r", speedtest_stop());         
  	        
  	        speedtest_start(); //time start //deserial test  	         
  	        
	        int z=0;		
	        int id=-1;
	        int wr=0;
	        
	        //FillSignalIndex(); //fill the index of loaded  signals

	                printf("\n\r ================================ *Value OR ExState* ====================================\n\r");	        
	        for (z=0; z < MAX_Signals; z++) {
//////	              printf(" \n\r |Signal FIELDS BEFORE parser: Name{%s} Val0[%i]  Val1[%i]| \n\r",Signal_Array[z].Name, Signal_Array[z].Value[0] , Signal_Array[z].Value[1]); //DEBUG
	              int test=0;	            
	              char buffer[350]="";
	              
	              strcpy (buffer, Signal_Array[z].Name);
	              test = unpack_signal(buffer  ,z); //from buffer to signal with number Z
	              
	             if (DEBUG == 1)  printf ("FROM SRV ->> [#%i]  Name:[%s]   Val:[%i]     Ex[%i] \n\r",z,Signal_Array[z].Name,Signal_Array[z].Value[1],Signal_Array[z].ExState);	                    
	
    		    }
	                printf(" ================================ *** ====================================\n\r");    		    
                 
		      printf(" ==>   SPEEDTEST Deserial signals signals: [ %ld ] ms. \n\r", speedtest_stop());     

         	        printf("=================== ==>  Calculate all HASH TIME START ============================= \n\r", speedtest_stop());         
	        speedtest_start(); //time start
	        FillHash(); //fill the array of hash
	        printf("=================== ==>  Calculate all HASH TIME: [ %ld ] ms. \n\r", speedtest_stop());         
         break;
                     //=========  SEND all 485 signals to TCPCache =======

      speedtest_start ();       //time start
      int x = 0;
//     socket_init();


      strcpy (message, "");     //erase buffer
      //printf("1Must be empty buffer - MESSAGE:[%s] \n\r",message);
      char tmpz[150];
      for (x = 0; x < MAX_Signals; x++)
        {
          if (strlen (Signal_Array[x].Name) > 1)
            {                   //write if Name not empty
//          socket_init();
              strcpy (packed_txt_string, "");   //erase buffer
              //printf("2Must be empty buffer - Packed_txt_string:[%s] \n\r",packed_txt_string);

              // PIZDEC ---              
              pack_signal (x, tmpz);
              // end pizdec

              //printf("After sSerial_by_num [%s] \n\r",packed_txt_string);
              //printf("After pack_signal [%s] \n\r",tmpz);
              strcat (message, tmpz);
              //printf("[%i] construct MESSAGE:[%s] \n\r",x,message);
//          socket_close();
            }
          else
            break;              // signals list is end
        }
      strcpy (tst, "");
      frame_pack ("wr", message, tst);
      tcpresult = frame_tcpreq (tst);
      if (DEBUG == 1) printf ("\n\r SEND TST^[%s] \n\r", tst);
      printf ("Status of TCP SEND: [%i]\n\r", tcpresult);
      //tcpsignal_packet_write(message);
      // printf("Send to TCPCache:[%s] \n\r",message);


         
        printf("end iteration.. \n\r");     
       }

//*************CLOSE SOCKET***************
 socket_close();


return 0;
}
