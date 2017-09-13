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
#include <errno.h>

int virt_mb_ReadtoCache(int dIndex,  int reg_count){ //read from real devices to cache by virtual devices Index and regcount
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[VirtDevRegs];
     int rc;
     int i;
     int fl;
                      
       ctx = modbus_new_rtu("/dev/ttySP0", 115200, 'N', 8, 1);
                           
       if (ctx == NULL) {
           fprintf(stderr, "Unable to create the libmodbus context\n");
           return -1;
        }
        struct timeval old_response_timeout;
        struct timeval response_timeout;
        struct timeval byte_timeout;
        
        /* Save original timeout */
        modbus_get_response_timeout(ctx, &old_response_timeout);
        
        /* Define a new and too short timeout! */
        /* response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        */
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
      
	int ID;
	ID = Device_Array[dIndex].MB_Id;
	printf ("ID from virtdev list %i \n\r",ID);
       modbus_set_slave(ctx, ID);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection failed %i\n",ID);
       if(connected == 0)
       printf("connected %i\n",ID);
       int cn=0;
	   
       rc = modbus_read_registers(ctx, 0, reg_count, tab_reg);
                                                                                               
          if (rc == -1) {
              printf("[MB_ID #%i]  Connection failed !\n",ID);
              fprintf(stderr, "MB_READ: %s\n", modbus_strerror(errno));
              return -1;
            } 
            
             
    //strcpy(Device_Array[dIndex].Name,Name);
    //Device_Array[dIndex].MB_Id=ID;                                 //Modbus device ID
     int cx=0;
     for (cx=0; cx < reg_count; cx++){
        Device_Array[dIndex].MB_Registers[cx]=tab_reg[cx];     //Mb register number
       }
        


     usleep(5 * 1000); //delay for Mod bus restore functional     

     modbus_flush(ctx);
    
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?

     
return tab_reg[0];
}


int virt_mb_CachetoDev(int dIndex,  int reg_count){ //read from VIRTUAL devices to REAL devices 
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[VirtDevRegs];
     int rc;
     int i;
     int fl;
                      
       ctx = modbus_new_rtu("/dev/ttySP0", 115200, 'N', 8, 1);
                           
       if (ctx == NULL) {
           fprintf(stderr, "Unable to create the libmodbus context\n");
           return -1;
        }
        struct timeval old_response_timeout;
        struct timeval response_timeout;
        struct timeval byte_timeout;
        
        /* Save original timeout */
        modbus_get_response_timeout(ctx, &old_response_timeout);
        
        /* Define a new and too short timeout! */
        /* response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        */
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
      
	int ID;
	ID = Device_Array[dIndex].MB_Id;
	printf ("ID from virtdev list %i \n\r",ID);
       modbus_set_slave(ctx, ID);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection failed %i\n",ID);
       if(connected == 0)
       printf("connected %i\n",ID);
       int cn=0;
	   
       rc = modbus_write_registers(ctx, 0, reg_count, tab_reg);
                                                                                               
          if (rc == -1) {
              printf("[MB_ID #%i]  Connection failed !\n",ID);
              fprintf(stderr, "MB_READ: %s\n", modbus_strerror(errno));
              return -1;
            } 
            
             
    //strcpy(Device_Array[dIndex].Name,Name);
    //Device_Array[dIndex].MB_Id=ID;                                 //Modbus device ID
     int cx=0;
     for (cx=0; cx < reg_count; cx++){
        Device_Array[dIndex].MB_Registers[cx]=tab_reg[cx];     //Mb register number
       }
        


     usleep(5 * 1000); //delay for Mod bus restore functional     

     modbus_flush(ctx);
    
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?

     
return tab_reg[0];
}



int CheckBit(uint sigReg, int iBit ) {
int result;
 if ( ( sigReg & ( 1 << iBit)) !=0) 
     {result=1;}
                else result=0;
return result;
}

int virtdev_to_signals(void){ //from virtual devices to signals parser
int c,x;
for (c=0; c < VirtDev; c++){ // cycle for divices 
     
     for (x=0; x < MAX_Signals; x++){ //cycle for signals
       if ( Signal_Array[x].MB_Id == Device_Array[c].MB_Id ){ // if founded modbus id = virtual modbus ID
          
           if ( strstr ( Signal_Array[x].Val_Type,"int") !=NULL ){ // if value int
                Signal_Array[x].Value[1] = Device_Array[c].MB_Registers[ Signal_Array[x].MB_Reg_Num ];
            //    printf("INT SIGNAL [Name: %s] [Value:%i] \n\r",Signal_Array[x].Name, Signal_Array[x].Value[1]); //DEBUG
           }
           
           if ( strstr ( Signal_Array[x].Val_Type,"bit") !=NULL ){ // if value bit
              //get register from virt
              //check bit_pos and state of bit
              //return result to Signal.Value
              int reg;
              reg = Device_Array[c].MB_Registers[ Signal_Array[x].MB_Reg_Num ];
              
              Signal_Array[x].Value[1] = CheckBit (reg,Signal_Array[x].Bit_Pos); //register and bit position
          //      printf("BIT SIGNAL [Name: %s] [Value:%i] \n\r",Signal_Array[x].Name, Signal_Array[x].Value[1]);     //DEBUG  
           }
         }
      }
    }

return 0;
}

int main(int argc , char *argv[])
{

//INIT SIGNALS     
    printf("MAX_Signals [%i] \n",MAX_Signals);
    init_signals_list(); // erase signal lsit 
    if( socket_init("127.0.0.1") != 0) {
    printf ("NO Connection to server\n\r");
    return;    
    }
    
int tcpresult=0;
char tst[MAX_MESS];    
while (1){
	printf ("***THIS iS PcSense [MordorBUS]***\n\r");
	speedtest_start(); //time start
        strcpy (signal_parser_buf,""); //erase buffer for next iteration
	//======================== read all 485 signals from server create signals and virtual devices ===================
       strcpy(message,"");
       frame_pack("rd","485.kb.k",message);
       tcpresult = frame_tcpreq(message);
       printf("Status of TCP SEND: [%i]\n\r",tcpresult);
       if ( tcpresult > 1){
           frame_unpack(signal_parser_buf, tst);
          }
    
    /*      
	if ( tcpsignal_read("485.kb.k") == 0 ){ // if we get response from server
	    printf("--- LOADED SIGNALS FROM SERVER: \n\r {%s} \n\r",signal_parser_buf); // debug
            tcpsignal_parser(signal_parser_buf); //explode by ";"
            strcpy (signal_parser_buf,""); //erase buffer for next iteration
	    }
     */
//    socket_close();
      int signals_found=0;
       signals_found  = Data_to_sName (tst);
       
       
	printf("=================== ==>   SPEEDTEST Time load signals: [ %ld ] ms. \n\r", speedtest_stop());     
  
  
	 speedtest_start(); //time start
         int z=0;

         for (z=0; z < MAX_Signals; z++) {             

            //if ( sDeSerial_by_num (z) == 0){
            
             if ( signals_found > 0) {
//                print_by_name(Signal_Array[z].Name);
                virt_mb_filldev(Signal_Array[z].Name, Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num); //init device list
             } else break;
    
        }
        
        virt_mb_devlist(); //show virtdev list

        printf(" ==>   SPEEDTEST Deserial signals signals: [ %ld ] ms. \n\r", speedtest_stop());     
  
	speedtest_start(); //time start     
     // MODBUS CODES
     int c=0;
     int total_dev_regs=0;
     for (c=0; c < VirtDev; c++)
        {
          if  (Device_Array[c].MB_Id > 0 ){ //if mb device ID  > 0
          
               total_dev_regs = virt_mb_registers( c ); // get total registers count for virtual devices list
               printf ( "[MB_ID %i Regs %i] \n\r",Device_Array[c].MB_Id , total_dev_regs  );
	       virt_mb_ReadtoCache ( c , total_dev_regs); // read from real devices to virtual
	       
              }
        else break; //if MB_Id = 0  BREAK all
       }
    //  virt_mb_devlist(); //show virtdev list
      virtdev_to_signals(); //convert virtual devices to real signals
     // END MB
     printf("\n\rVirtDev to real signals:\n\r");
     z=0;
     for ( z=0; z < MAX_Signals; z++ ) { //at this point ALL FINE
           printf ("Parameters in field Name: \n\r" );
           printf ("TOrealSignals[%i] Name:[%s]  \n\r ", z, Signal_Array[z].Name);
           printf ("Parameters in Signal struct fields: \n\r" );           
           printf ("TOrealSignals[%i] Name:[%s] Value:[%i] ExState[%i]\n\r ", z, Signal_Array[z].Name, Signal_Array[z].Value[1] ,Signal_Array[z].ExState  );
           char buffer[350]="";
           int test=0;
            strcpy (buffer, Signal_Array[z].Name);
            test = unpack_signal(buffer  ,z); //from buffer to signal with number Z
            printf ("\n\r #%i RESTORED SIGNAL -  Name:[%s] Val:[%i] Ex[%i] \n\r",z,Signal_Array[z].Name,Signal_Array[z].Value[1],Signal_Array[z].ExState);  //DEBUG
                                                                       
          if ( Signal_Array[z].Value[1] > 0 ) printf ("[%i] >> Name:[%s] Value:[%i] \n\r ", z, Signal_Array[z].Name, Signal_Array[z].Value[1] );
          if ( Signal_Array[z].ExState > 0 ) printf  ("[%i] >> Name:[%s] ExState:[%i] \n\r ", z, Signal_Array[z].Name, Signal_Array[z].ExState );
     }
     
  printf(" ==>   SPEEDTEST Get MB_ID and REGS from signals: [ %ld ] ms. \n\r", speedtest_stop());          



     //=========  SEND all 485 signals to TCPCache =======

     speedtest_start(); //time start     
     int x=0;
//     socket_init();     

     
     strcpy(message,""); //erase buffer     
     printf("1Must be empty buffer - MESSAGE:[%s] \n\r",message);
     char tmpz[150];
     for (x=0; x < MAX_Signals; x++){
      if ( strlen (Signal_Array[x].Name) > 1 ){ //write if Name not empty
//          socket_init();
            strcpy(packed_txt_string,""); //erase buffer
            printf("2Must be empty buffer - Packed_txt_string:[%s] \n\r",packed_txt_string);
            
            // PIZDEC ---
            sSerial_by_num(x); //pack to serial prepare for send into global buffer packed_txt_string
            pack_signal(x,tmpz);
            // end pizdec
            
            printf("After sSerial_by_num [%s] \n\r",packed_txt_string);
            printf("After pack_signal [%s] \n\r",tmpz);
            strcat(message, tmpz);
            printf("[%i] construct MESSAGE:[%s] \n\r",x,message);
//          socket_close();
         } else break; // signals list is end
     }
       strcpy(tst,"");
       frame_pack("wr",message,tst);
       tcpresult = frame_tcpreq(tst);
       printf("\n\r SEND TST^[%s] \n\r",tst);
       printf("Status of TCP SEND: [%i]\n\r",tcpresult);
     //tcpsignal_packet_write(message);
    // printf("Send to TCPCache:[%s] \n\r",message);
    
     

      
      
  printf(" ++++++++++++++++++++++++==>   SPEEDTEST Send to TCPCache Time: [ %ld ] ms. \n\r", speedtest_stop());     
}
 socket_close();

return 0;
}
