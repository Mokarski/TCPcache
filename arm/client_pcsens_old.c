/*
 *   C ECHO client example using sockets
 */
#include <stdio.h>		//printf
#include <string.h>		//strlen
#include <sys/socket.h>		//socket
#include <arpa/inet.h>		//inet_addr
#include <stdlib.h>
#include "/home/opc/Kombain/test/include/modbus/modbus.h"

#define DEBUG 3 // may be set to 1,2,3,4
#include "network.h"
#include "signals.h"
#include "virtmb.h"
#include "speedtest.h"
#include <errno.h>



int
virt_mb_ReadtoCache (int dIndex, int reg_count)
{				//read from real devices to cache by virtual devices Index and regcount
  int connected;
  modbus_t *ctx;
  uint16_t tab_reg[VirtDevRegs];
  int rc;
  int i;
  int fl;
  int ret=0;
  ctx = modbus_new_rtu ("/dev/ttySP0", 115200, 'N', 8, 1);
  if ( DEBUG > 0 ) printf ("\n \r >>>> READ real device [%i] from position [0] Number register to read [%i] Rd[%i] Wr[%i]\n\r",Device_Array[dIndex].MB_Id,reg_count, Device_Array[dIndex].Rd, Device_Array[dIndex].Wr);
  if (ctx == NULL)
    {
      fprintf (stderr, "Unable to create the libmodbus context\n");
      ret=-1;
      return 4;
    }
  struct timeval old_response_timeout;
  struct timeval response_timeout;
  struct timeval byte_timeout;

  /* Save original timeout */
  modbus_get_response_timeout (ctx, &old_response_timeout);

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
  printf ("ID from virtdev list %i \n\r", ID);
  modbus_set_slave (ctx, ID);
  connected = modbus_connect (ctx);

  if (connected == -1)
    {
     printf ("Connection failed %i\n", ID);
     ret=4; 
     return ret;
         
    }
    
  if (connected == 0){
    printf ("connected %i\n", ID);
    ret=0;
    }
  int cn = 0;

  rc = modbus_read_registers (ctx, 0, reg_count, tab_reg);

  if (rc == -1)
    {
      printf ("[MB_ID #%i]  Connection failed !\n", ID);
      fprintf (stderr, "MB_READ: %s\n", modbus_strerror (errno));
      ret=4;
      return ret;
    } else ret = 3;


  //strcpy(Device_Array[dIndex].Name,Name);
  //Device_Array[dIndex].MB_Id=ID;                                 //Modbus device ID
  int cx = 0;
  for (cx = 0; cx < reg_count; cx++)
    {
      Device_Array[dIndex].MB_Registers[cx] = tab_reg[cx];	//Mb register number
    }



  usleep (5 * 1000);		//delay for Mod bus restore functional     

  modbus_flush (ctx);

  modbus_close (ctx);
  modbus_free (ctx);		//close COM ?


  return ret;
}


int
virt_mb_CachetoDev (int dIndex, int reg_count)
{				//read from VIRTUAL devices to REAL devices 
  int connected;
  modbus_t *ctx;
  uint16_t tab_reg[VirtDevRegs];
  int rc;
  int i;
  int fl;
  int ret;

  ctx = modbus_new_rtu ("/dev/ttySP0", 115200, 'N', 8, 1);

  if (ctx == NULL)
    {
      fprintf (stderr, "Unable to create the libmodbus context\n");
      return 4;
      ret =4;
    }
  struct timeval old_response_timeout;
  struct timeval response_timeout;
  struct timeval byte_timeout;

  /* Save original timeout */
  modbus_get_response_timeout (ctx, &old_response_timeout);

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
  printf ("WR ID[%i] from virtdev list, regs to write [%i] \n\r", ID, reg_count);
  modbus_set_slave (ctx, ID);
  connected = modbus_connect (ctx);

  if (connected == -1){
      printf ("WR - Connection failed %i\n", ID);
      ret=4;
     }
    
  if (connected == 0){
      printf ("WR - connected %i\n", ID);
      ret=4;
     }
     
  int cn = 0;

  int n;
  for (n=0; n < reg_count; n++ ){ //copy virt dev registers to tab_reg
      //tab_reg[n] = Device_Array[dIndex].WR_MB_Registers[n] | Device_Array[dIndex].MB_Registers[n];
      tab_reg[n] = Device_Array[dIndex].WR_MB_Registers[n];
      printf (">>>>>>>>>>>>>>>>>>>>>>>   WR----REG[%i] = %i \n\r",n,Device_Array[dIndex].WR_MB_Registers[n]);
      //printf("=========================>>>> WRite REGS[%i] = %i Read REG=%i Or REG= %i \n\r ",n,Device_Array[dIndex].WR_MB_Registers[n],Device_Array[dIndex].MB_Registers[n], Device_Array[dIndex].WR_MB_Registers[n]|Device_Array[dIndex].MB_Registers[n]);
  }
 
    rc = modbus_write_registers (ctx, 0, reg_count, tab_reg);
//  rc = modbus_write_registers (ctx, 0, reg_count, Device_Array[dIndex].WR_MB_Registers);

  if (rc == -1)
    {
      printf ("WR [MB_ID #%i]  Connection failed !\n", ID);
      fprintf (stderr, "WR MB_READ: %s\n", modbus_strerror (errno));
      ret=4;
      return ret;
    } else ret=3; //3 - executed OK!


  //strcpy(Device_Array[dIndex].Name,Name);
  //Device_Array[dIndex].MB_Id=ID;                                 //Modbus device ID
  int cx = 0;
  for (cx = 0; cx < reg_count; cx++)
    {
      Device_Array[dIndex].MB_Registers[cx] = tab_reg[cx];	//Mb register number
    }



  usleep (5 * 1000);		//delay for Mod bus restore functional     

  modbus_flush (ctx);

  modbus_close (ctx);
  modbus_free (ctx);		//close COM ?


  return ret;
}



int
CheckBit (uint sigReg, int iBit)
{
  int result;
  if ((sigReg & (1 << iBit)) != 0)
    {
      result = 1;
    }
  else
    result = 0;
  return result;
}

int virtdev_to_signals (void)
{				//from virtual devices to signals
  int c, x;
  for (c = 0; c < VirtDev; c++)
    {				// cycle for divices 

      for (x = 0; x < MAX_Signals; x++)
	{			//cycle for signals
	  if (Signal_Array[x].MB_Id == Device_Array[c].MB_Id)
	    {			// if founded modbus id = virtual modbus ID

	      if (strstr (Signal_Array[x].Val_Type, "int") != NULL)
		{		// if value int
		  Signal_Array[x].Value[1] = Device_Array[c].MB_Registers[Signal_Array[x].MB_Reg_Num];
		  Signal_Array[x].ExState = Device_Array[c].ExState; // for write device connecton state
		  //    printf("INT SIGNAL [Name: %s] [Value:%i] \n\r",Signal_Array[x].Name, Signal_Array[x].Value[1]); //DEBUG
		}

	      if (strstr (Signal_Array[x].Val_Type, "bit") != NULL)
		{		// if value bit
		  //get register from virt
		  //check bit_pos and state of bit
		  //return result to Signal.Value
		  int reg;
		  reg =  Device_Array[c].MB_Registers[Signal_Array[x].MB_Reg_Num];
		  Signal_Array[x].ExState = Device_Array[c].ExState; //for write ExState back to Signals.ExState
		  Signal_Array[x].Value[1] = CheckBit (reg, Signal_Array[x].Bit_Pos);	//register and bit position
		  //      printf("BIT SIGNAL [Name: %s] [Value:%i] \n\r",Signal_Array[x].Name, Signal_Array[x].Value[1]);     //DEBUG  
		}
	    }
	}
    }

  return 0;
}

int signals_to_virtdev (void)
{				//from virtual devices to signals
  int c, x;
  for (c = 0; c < VirtDev; c++)
    {				// cycle for divices 

      for (x = 0; x < MAX_Signals; x++)
	{			//cycle for signals
	  if ( (Signal_Array[x].MB_Id == Device_Array[c].MB_Id) && (Signal_Array[x].ExState ==2 ) ) //ExState = 2 signal to write
	    {			// if founded modbus id = virtual modbus ID

	      if (strstr (Signal_Array[x].Val_Type, "int") != NULL)
		{		// if value int

		  Device_Array[c].WR_MB_Registers[Signal_Array[x].MB_Reg_Num] = Signal_Array[x].Value[1];
		  Device_Array[c].Wr = Signal_Array[x].ExState; // for write device marker
		  //    printf("INT SIGNAL [Name: %s] [Value:%i] \n\r",Signal_Array[x].Name, Signal_Array[x].Value[1]); //DEBUG
		}

	      if (strstr (Signal_Array[x].Val_Type, "bit") != NULL)
		{		// if value bit
		  //get register from virt
		  //check bit_pos and state of bit
		  //return result to Signal.Value
		  
		  Device_Array[c].Wr = Signal_Array[x].ExState; // for write device marker
		  int reg;
		  reg =  Device_Array[c].WR_MB_Registers[Signal_Array[x].MB_Reg_Num]; //take before write register state		  
                  Device_Array[c].WR_MB_Registers[Signal_Array[x].MB_Reg_Num] = bit_mask(Signal_Array[x].Value[1],Signal_Array[x].Bit_Pos,reg);
                  
		  //      printf(">> WR BIT SIGNAL [Name: %s] [Value:%i] \n\r",Signal_Array[x].Name, Signal_Array[x].Value[1]);     //DEBUG  
		}
	    }
	}
    }

  return 0;
}



//*********************************Read_Op ****************************************
int Read_Op(){



retrun 0;
}
//*********************************************************************************


//*********************************Write_Op ****************************************
int Write_Op(){



retrun 0;
}
//*********************************************************************************
int main (int argc, char *argv[])
{
  int TCP_SEND;
  int tcpresult = 0;
  char tst[MAX_MESS];
  int LoadList=0; //flag first load list
  int MB_Write=0; //flag modbus write
//INIT SIGNALS     
if (DEBUG == 1)   printf ("MAX_Signals [%i] \n", MAX_Signals);
  init_signals_list ();		// erase signal lsit 
  if (socket_init ("127.0.0.1") != 0)
    {
      printf ("NO Connection to server\n\r");
      return;
    }


  while (1)
    {
      MB_Write=0;
      printf ("***THIS iS PcSense [MordorBUS]***\n\r");
      //speedtest_start ();	//time start
      strcpy (signal_parser_buf, "");	//erase buffer for next iteration
      //======================== read all 485 signals from server create signals and virtual devices ===================
      strcpy (message, "");
      frame_pack ("rd", "485.", message);
      tcpresult = frame_tcpreq (message);
      if (DEBUG == 3) printf ("Status of TCP SEND Read request: [%i][%s]\n\r", tcpresult,message);
      if (tcpresult > 1)
	{
	  frame_unpack (signal_parser_buf, tst);
	} else printf("!!!ERROR freame_tcpreq = %i \n\r",tcpresult);

      /*      
         if ( tcpsignal_read("485.kb.k") == 0 ){ // if we get response from server
         printf("--- LOADED SIGNALS FROM SERVER: \n\r {%s} \n\r",signal_parser_buf); // debug
         tcpsignal_parser(signal_parser_buf); //explode by ";"
         strcpy (signal_parser_buf,""); //erase buffer for next iteration
         }
       */
//    socket_close();
      int signals_found = 0;
      signals_found = Data_to_sName (tst); //extract all signals from buffer and put into Signal.Name field


     // printf	("=================== ==>   SPEEDTEST Time load signals: [ %ld ] ms. \n\r",	 speedtest_stop ());


      speedtest_start ();	//time start
      int z = 0;
      
      for (z = 0; z < MAX_Signals; z++) //create virt devices and unpack signals to struct Signal_Array[]
	{
	
	  char buffer[350] = "";
	  int test = 0;
	  strcpy (buffer, Signal_Array[z].Name);
	  //printf ("#%i NameField {%s} \n\r",z,Signal_Array[z].Name);
	  test = unpack_signal (buffer, z);	//UnPACK Signal from buffer to signal with number Z
	  //printf ("\n\r #%i RESTORED SIGNAL -  Name:[%s] Val:[%i] Ex[%i] \n\r",z,Signal_Array[z].Name,Signal_Array[z].Value[1],Signal_Array[z].ExState);  //DEBUG
	  
	  //****************** DEBUG PRINT **************************

	  if ( (Signal_Array[z].Value[1] > 0) || (Signal_Array[z].ExState > 0) ) {  
	        if ( DEBUG == 1 ) printf ("[%i]From_SRV  -->> Name:[%s] Value:[%i] ExState:[%i]\n\r ", z,Signal_Array[z].Name, Signal_Array[z].Value[1], Signal_Array[z].ExState);
	       }
	       
	  if ( Signal_Array[z].ExState > 0 ) { //flag to SRV send 
	                TCP_SEND = 1;          //flag to SRV send 
	           }
	           
	  if ( Signal_Array[z].ExState == 0 ) {	           //debug
	            //    TCP_SEND = 0;
	                if ( DEBUG == 1 )  
	                     //if ( strstr (Signal_Array[z].Name, "485.kb.kei") != NULL) Signal_Array[z].ExState =1; //for keyboard only
	                     printf ("[%i]From_SRV EX=0 -->> Name:[%s] Value:[%i] ExState:[%i]\n\r ", z,Signal_Array[z].Name, Signal_Array[z].Value[1], Signal_Array[z].ExState);
	               }
	               
	  if ( Signal_Array[z].ExState  == 1 ) {	           //debug
	            //    TCP_SEND = 0;
	                if ( DEBUG == 1 )  
	                     //if ( strstr (Signal_Array[z].Name, "485.kb.kei") != NULL) Signal_Array[z].ExState =1; //for keyboard only
	                     printf ("[%i]From_SRV EX=1  -->> Name:[%s] Value:[%i] ExState:[%i]\n\r ", z,Signal_Array[z].Name, Signal_Array[z].Value[1], Signal_Array[z].ExState);
	               }
	       
	  if ( Signal_Array[z].ExState == 2 ) {  
	        if ( DEBUG == 2 ) printf ("[%i]>>>> Write From_SRV EX=2 -->> Name:[%s] Value:[%i] ExState:[%i] MB_ID[%i] MB_REG[%i]\n\r ", z,Signal_Array[z].Name, Signal_Array[z].Value[1], Signal_Array[z].ExState,Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num );
	       }
	 // if (Signal_Array[z].ExState > 0)
	 //          printf ("[%i]FromSRV NOW -->> Name:[%s] ExState:[%i] \n\r ", z,Signal_Array[z].Name, Signal_Array[z].ExState);
         //****************** END DEBUG PRINT ***************************************
         
        //if ( LoadList ==0 ){
             virt_mb_filldev (Signal_Array[z].Name, Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num, Signal_Array[z].ExState );	//init virtual device list and copy ExState
        
        //    }

	  if (signals_found > 0)
	    {
//            print_by_name(Signal_Array[z].Name);
              if (Signal_Array[z].ExState == 2 ) {  //flag to read or write
                  MB_Write = 1;
	          if ( DEBUG > 0 ) {
	              if (Signal_Array[z].ExState == 2 ) printf(">>>> WRITE = Name [%s] MB_ID{%i} MB_reg[%i] Ex{%i} VAL{%i} \n\r ",Signal_Array[z].Name, Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num, Signal_Array[z].ExState, Signal_Array[z].Value[1] );
	             }
	          
	          }
	    }
	      else  {  
	               printf("signals_found = 0\n\r");
	               break;
	             }

	} //end for
	
	//LoadList=1; //first signals loaded and configure virtual modbas devices

     // virt_mb_devlist ();	//show virtdev list

      printf (" ==>   SPEEDTEST Deserial signals: [ %ld ] ms. \n\r", speedtest_stop ());
	      

      signals_to_virtdev(); //signals to write put bit to virt device register
      speedtest_start ();	//time start     
      //************************************************************************************ MODBUS CODES
      int c = 0;
      int total_dev_regs = 0;
      //printf ("Device_Array[c].MB_Id ", Device_Array[c].MB_Id );
      for (c = 0; c < VirtDev; c++)
	{
	 // printf ("Device_Array[c].MB_Id %i\n\r ", Device_Array[c].MB_Id );
	 // printf ("[Device_Array MB_ID {%i} Register_counter {%i}] \n\r", Device_Array[c].MB_Id, total_dev_regs);
	  if (Device_Array[c].MB_Id > 0)
	    {			//if mb device ID  > 0

	      total_dev_regs = virt_mb_registers (c);	// get total registers count for virtual devices list
	//    printf ("[MB_ID %i Regs %i] \n\r", Device_Array[c].MB_Id, total_dev_regs);
	      //if ( total_dev_regs  > 1 ) total_dev_regs = total_dev_regs+1; //fix 1 position miss
	      
	      if (Device_Array[c].Rd == 1){ // separate write and read registers!!! 
	          Device_Array[c].Rd =0;
	          Device_Array[c].ExState = virt_mb_ReadtoCache (c, total_dev_regs );	// read from real devices to virtual and Write ExState to virtual device/ if ExState = 4 or -1 ->  error connection	      
	          if ( DEBUG == 1 ) printf ("READ FROM DEVICE ID[%i] Total_REGS[%i] \n\r",Device_Array[c].MB_Id,total_dev_regs); // if signals empty 
	          }

              if (Device_Array[c].Wr == 2){ // separate write and read registers!!!
                  if ( DEBUG == 1 ) printf(" ====================================================------>>> Have signal to write! \n\r");
                  if ( DEBUG == 1 ) printf (">>>> WRITE TO  DEVICE ID[%i] Total_REGS[%i] \n\r",Device_Array[c].MB_Id,total_dev_regs); // if signals empty 
                  Device_Array[c].ExState = virt_mb_CachetoDev (c, total_dev_regs);	// Write to Modbus real devices
                  Device_Array[c].Wr = 0;
                 }
	    }
	     //else
	         // break;		//if MB_Id = 0  BREAK all ???????
	} //end for
	
       if ( DEBUG == 1)    virt_mb_devlist ();	//show virtdev list
       if ( DEBUG == 2)    virt_mb_devlist ();	//show virtdev list
       //virt_mb_devlist ();	//show virtdev list
     
      //break;
      LoadList++;
      //printf("Konec %i \n\r",LoadList);
      //if ( LoadList == 3 )       break; //***********
      
      virtdev_to_signals ();	//convert virtual devices to real signals
      //***********************************************************************************  END MB

       printf ("\n\rVirtDev to real signals:\n\r");
     

      printf
	(" ==>   SPEEDTEST Get MB_ID and REGS from signals: [ %ld ] ms. \n\r",
	 speedtest_stop ());



//========++++++++++++++++++++++++++=  SEND all 485 signals to TCPCache =======+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      //TCP_SEND
      
      speedtest_start ();	//time start     
      int x = 0;
//     socket_init();     
      int ready_to_send_tcp=0;

      strcpy (message, "");	//erase buffer     
      char tmpz[150];
      for (x = 0; x < MAX_Signals; x++)
	{
	  if ( (Signal_Array[x].Value[1] > 0) || (Signal_Array[x].ExState > 0) )
	          if ( DEBUG == 3 )  printf ("[%i]TO_SRV  <<-- Name:[%s] Value:[%i] ExState:[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState);
	          
	  if  (Signal_Array[x].Value[1] > 0) 
	         if ( DEBUG == 1 )  printf ("[%i]TO_SRV  <<-- Name:[%s] Value:[%i] ExState:[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState);
	         
	if  (Signal_Array[x].Value[1] > 0) 
	         if ( DEBUG == 4 )  printf ("[%i]TO_SRV  <<-- Name:[%s] Value:[%i] ExState:[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState);
	         
	  if (strlen (Signal_Array[x].Name) > 1) //write if Name not empty
	    {		
	      pack_signal (x, tmpz);
	      strcat (message, tmpz);
	      ready_to_send_tcp=1;
            }
	  else
	    break;		// signals list is end
	}
	
      strcpy (tst, "");
      frame_pack ("wr", message, tst);
      if (TCP_SEND > 0 )  {
          if (ready_to_send_tcp == 1) tcpresult = frame_tcpreq (tst); //send to srv
         }
      
      if (DEBUG == 3) printf ("\n\r SEND WRITE request TST^[%s] \n\r", tst);
      printf ("Status of TCP SEND: [%i]\n\r", tcpresult);
      printf
	(" ++++++++++++++++++++++++==>   SPEEDTEST Send to TCPCache Time: [ %ld ] ms. \n\r",	 speedtest_stop ());
    // break;//debug
    }
  socket_close ();

  return 0;
}
