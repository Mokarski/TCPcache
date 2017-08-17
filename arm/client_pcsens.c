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
    socket_init();    
    
while (1){

speedtest_start(); //time start

//======================== read all 485 signals from server create signals and virtual devices ===================

    if ( tcpsignal_read(".") == 0 ){ // if we get response from server
        tcpsignal_parser(signal_parser_buf);
    }
//    socket_close();
    
  printf("=================== ==>   SPEEDTEST Time load signals: [ %ld ] ms. \n\r", speedtest_stop());     
  
  
 speedtest_start(); //time start
    int z=0;

       for (z=0; z < MAX_Signals; z++) {

            if ( sDeSerial_by_num (z) == 0){
//                print_by_name(Signal_Array[z].Name);
                virt_mb_filldev(Signal_Array[z].Name, Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num); //init device list
             } else break;
        }
     //virt_mb_devlist(); //show virtdev list

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
//      virt_mb_devlist(); //show virtdev list
      virtdev_to_signals();
     // END MB
     
     
  printf(" ==>   SPEEDTEST Get MB_ID and REGS from signals: [ %ld ] ms. \n\r", speedtest_stop());          



     //=========  SEND all 485 signals to TCPCache =======

speedtest_start(); //time start     
     int x=0;
//     socket_init();     

     
     strcpy(message,""); //erase buffer     
     printf("Packed_txt_string:[%s] \n\r",packed_txt_string);
     for (x=0; x < MAX_Signals; x++){
      if ( strlen (Signal_Array[x].Name) > 1 ){ //write if Name not empty
//          socket_init();
            strcpy(packed_txt_string,""); //erase buffer
            sSerial_by_num_short(x);
            strcat(message, packed_txt_string);
            //printf("message part:[%s] \n\r",message);
//          socket_close();
         } else break; // signals list is end
     }
     
     tcpsignal_packet_write(message);
     printf("Send to TCPCache:[%s] \n\r",message);
    
     

      
      
  printf(" ++++++++++++++++++++++++==>   SPEEDTEST Send to TCPCache Time: [ %ld ] ms. \n\r", speedtest_stop());     
}
 socket_close();

return 0;
}
