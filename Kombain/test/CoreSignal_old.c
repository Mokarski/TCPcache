#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "/home/opc/Kombain/test/include/modbus/modbus.h"

#define MAX_Signals 100
#define STR_LEN_TXT 100
#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS        0
#define NUM_THREADS 2

struct timeval tv1,tv2,dtv;
struct timezone tz;

void time_start() {
gettimeofday(&tv1,&tz);
}

long time_stop(){
gettimeofday(&tv2,&tz);
dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
   if(dtv.tv_usec<0) 
   { 
     dtv.tv_sec--; dtv.tv_usec+=1000000; 
   }
 return dtv.tv_sec*1000+dtv.tv_usec/1000;
}

struct Signal {
 char Name[100];       //Name signal
 int  MB_Id;           //Modbus device ID
 int  MB_Reg_Num;      //Mb register number
 char Val_Type[10];    //type of value int or bit 
  int  Bit_Pos;        //bit position
  int  Value[2];        //value 2 int 
  char Reserv1[10];    //type of value int or bit
  char Reserv2[10];    //type of value int or bit
  char Reserv3[10];    //type of value int or bit
  int Off;            //Signal not used if OFF = 1;
  int ExState;        // 1-for execution,2-executed,3-expired,4-dropped 
  int Ex_HW;   //Execution Hardware mark PcProc =1 Panel 43=2 Panel 10=3 | soft_class 
  int Ex_SF;   //Execution Software mark ModBusMaster_RTU =1 Modbus_Master_TCP=2 CoreSignal=100
  int Prio;    //Priority of signal 0 - no priority
};
struct Signal Signal_Array[MAX_Signals];


struct Reg {
int Mb_Reg;
int Mb_Bit;
} Reg_Bit;

typedef struct someArgs_tag {
  int id;
  const char *msg;
  int out;
  int counters;
  struct Signal *SA_ptr;
} someArgs_t;

//void* threadFunc_Mb(void* data){
//pthread_exit(0);
//}

void* helloWorld2(void *args) {
someArgs_t *arg = (someArgs_t*) args;
    int len;
      printf("AGGRRR %i\n", arg->out);
 return SUCCESS;
}

void* helloWorld(void *args) {
  someArgs_t *arg = (someArgs_t*) args;
  int len;
      arg->out=1;
      arg->msg="123456";
      arg->SA_ptr[arg->counters].ExState=2;
//      arg->counters= len++;
      
      printf("PTR SA: %s\n",arg->SA_ptr[arg->counters].Name);
      printf("AGGRRR %i\n", arg->out);
      printf("MSG %s\n", arg->msg);
  return SUCCESS;
}

void init_list(void)
{
 register int t;
 for(t=0; t<MAX_Signals; ++t) Signal_Array[t].Name[0] = '\0';
}

void list (void)
{
register int t;
for (t=0; t<MAX_Signals; ++t) {
if(Signal_Array[t].Name[0]) 
  {
   printf("Signal Name: %s\n", Signal_Array[t].Name);
   Signal_Array[t].MB_Id=atoi(Signal_Array[t].Reserv1);
   printf("MB ID: %i\n", Signal_Array[t].MB_Id);
   
   Signal_Array[t].MB_Reg_Num=atoi(Signal_Array[t].Reserv2);
   printf ("MB Register Num: %i\n", Signal_Array[t].MB_Reg_Num);
   Signal_Array[t].Bit_Pos=atoi(Signal_Array[t].Reserv3);
   printf ("MB Register BitNum: %i\n", Signal_Array[t].Bit_Pos);
   printf ("Value type: %s\n", Signal_Array[t].Val_Type);
//     printf ("Reserve1: %s\n", Signal_Array[t].Reserv1); //reserved for file input conversion
//     printf ("Reserve2: %s\n", Signal_Array[t].Reserv2);
//     printf ("Reserve3: %s\n", Signal_Array[t].Reserv3);
     
     printf ("Signal OFF: %i\n", Signal_Array[t].Off);
     printf ("Execution flow state: %i\n", Signal_Array[t].ExState);
     printf ("Hardware execution mark: %i\n", Signal_Array[t].Ex_HW);
     printf ("Software execution mark: %i\n", Signal_Array[t].Ex_SF);
     printf ("Priority mark: %i\n", Signal_Array[t].Prio);

   printf ("Values: [Hi: %i] [Low: %i]\n\n", Signal_Array[t].Value[0],Signal_Array[t].Value[1]);
  }
}
printf("\n\n");
}

//kb.kbl.stop_oil_pump, bit, 31, 1, 0

int text_parser(char *str_in, char str_out1[STR_LEN_TXT], char str_out2[STR_LEN_TXT], char str_out3[STR_LEN_TXT], char str_out4[STR_LEN_TXT], char str_out5[STR_LEN_TXT])
{
 int i=0;
  i=strlen (str_in);
 int x=0;
 int block1=0; //block counter
 int block2=0;
 int block3=0;
 int block4=0;
 int block5=0;

 int  complete1=0; //flag to fill block
 int  complete2=0;
 int  complete3=0;
 int  complete4=0;
 int  complete5=0;
 
// printf("START PARSER \n");
 while ( x < i ) 
 {
    if ( (str_in[x]!=',') && (complete1 == 0) )
      {
        str_out1[block1]=str_in[x];
        block1++;
      }else       
      {
        if ( (str_in[x]==',') && (complete1 == 0) )
        {
          x++; //add +1 in counter etc next symbol
          complete1=1;
//          printf ("string str_out1 %s\n",str_out1);  
        }
      }
      
// printf ("complete1 %i\n",complete1);
      if ((complete1 == 1) && (complete2 == 0))
      {
    //   printf("Blohh 2\n");
        if (str_in[x]!=',')
         {
    //      printf("INPUT %c \n",str_in[x]);
          str_out2[block2]=str_in[x];
          block2++;
         }else
         {
          if (str_in[x]==',')
          {
           x++; //add +1 in counter etc next symbol
           complete2=1;
//           printf ("str_out2 %s\n",str_out2);
          }
         }
      
      }

      if ((complete2 == 1) && (complete3 == 0))
      {
        if (str_in[x]!=',')
         {
          str_out3[block3]=str_in[x];
          block3++;
         }else
         {
          if (str_in[x]==',')
          {
           x++; //add +1 in counter etc next symbol
           complete3=1;
//           printf ("str_out3 %s\n",str_out3);
          }
         }
      
      }


      if ((complete3 == 1) && (complete4 == 0))
      {
        if (str_in[x]!=',')
         {
          str_out4[block4]=str_in[x];
          block4++;
         }else
         {
          if (str_in[x]==',')
          {
           x++; //add +1 in counter etc next symbol
           complete4=1;
//           printf ("str_out4 %s\n",str_out4);
          }
         }
      
      }
      
      if ((complete4 == 1) && (complete5 == 0))
      {
        if (str_in[x]!=',')
         {
          str_out5[block5]=str_in[x];
          block5++;
         }else
         {
          if (str_in[x]==',')
          {
           //x++; //add +1 in counter etc next symbol
           complete5=1;
//           printf ("str_out5 %s\n",str_out5);
          }
          if (str_in[x]=='\0')
          {
           //x++; //add +1 in counter etc next symbol
           complete5=1;
//           printf ("str_out5 %s\n",str_out5);
          }
         }
      
      }

      
  x++;
 }
// printf("END PARSER \n");
//       printf ("string str_out1 %s\n",str_out1);  
//       printf ("string str_out2 %s\n",str_out2);  
//       printf ("string str_out3 %s\n",str_out3);  
//       printf ("string str_out4 %s\n",str_out4);  
//       printf ("string str_out5 %s\n",str_out5);  

return 0;
}

int file_load(void )
{
 FILE *fp;
 char str[128];
 char *row;
 int c;
  printf ("Load signals from signals.cfg\n");
 fp = fopen ("signals.cfg","r");
 if (fp == NULL) {printf ("error open file\n"); return -1;}
 else printf("Read from file next signals:\n");
    c=0;
 
 while (1)
 {
   row = fgets (str,sizeof(str),fp);
   if (row == NULL)
     {
      if ( feof (fp) !=0 )
      {
       printf ("Read signals from file,complete.\n");
       break;
      }
      else
      {
       printf ("\n ERROR: read from file\n");
       break;
      }
     }
    printf ("    %s",str); //string from file
    text_parser(str,Signal_Array[c].Name,Signal_Array[c].Val_Type,Signal_Array[c].Reserv1,Signal_Array[c].Reserv2,Signal_Array[c].Reserv3);
    //strcpy(Signal_Array[c].Name,str);
    c++;
  }
 printf ("Close file: ");
 if ( fclose(fp) == EOF) printf ("error close file \n");
    else printf ("complete\n");
    

  printf ("Loaded Signals: \n");
 
return 0;
}

int signal_update (char sName[100],int sVal){ //function to update parameters in tcp_cache_server list
int n=0;
int cnt=0;
  for (n=0;n<MAX_Signals;n++)
  {
   if (strstr(Signal_Array[n].Name,sName) != NULL) 
      {
        Signal_Array[n].Value[1]=sVal;
        cnt++;
        break;
      } 
  }
 return cnt; //if zerro then not found signals else number of matches
}

int get_reg_bit (char sName[100]){ //function to update parameters in tcp_cache_server list
int n=0;
int cnt=0;
  for (n=0;n<MAX_Signals;n++)
  {
   if (strstr(Signal_Array[n].Name,sName) != NULL) 
      {
        Reg_Bit.Mb_Reg=Signal_Array[n].MB_Reg_Num;
        Reg_Bit.Mb_Bit=Signal_Array[n].Bit_Pos;
        cnt++;
        break;
      } 
  }
 return cnt; //if zerro then not found signals else number of matches
}

int signal_read(char sName[100]){
int n=0;
int result=0;
  for (n=0;n<MAX_Signals;n++)
  {
   if (strstr(Signal_Array[n].Name,sName) != NULL) 
      {
        result=Signal_Array[n].Value[1];
        break;
      } 
  }
 return result; //if zerro then not found signals else return value
}




int CheckBit(uint sigReg, int iBit ) {
int result;
 if ( ( sigReg & ( 1 << iBit)) !=0) 
    {result=1;}
               else result=0;
return result;
}

int test_read(int devId,int regCount, int regNum, int bit){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
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
        
        /* Save original timeout */
        modbus_get_response_timeout(ctx, &old_response_timeout);
        
        /* Define a new and too short timeout! */
        response_timeout.tv_sec = 80;
        response_timeout.tv_usec = 80;
        modbus_set_response_timeout(ctx, &response_timeout);
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
                                                       
       modbus_set_slave(ctx, devId);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
       int cn=0;
       while(cn!=100)
       {
          rc = modbus_read_registers(ctx, 0, regCount, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
            for (i=0; i < rc; i++) {
                printf("reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
               }
           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     modbus_close(ctx);
     modbus_free(ctx);
     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
return fl;
}

int test_write(int devId,int regCount, int regNum, int bit){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[1];
     int rc;
     int i;
     int fl;
                      
       ctx = modbus_new_rtu("/dev/ttySP0", 115200, 'N', 8, 1);
                           
       if (ctx == NULL) {
           fprintf(stderr, "Unable to create the libmodbus context\n");
           return -1;
        }
                                                       
       modbus_set_slave(ctx, devId);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
       int cn=0;
       tab_reg[0]=1;
       while(cn!=1)
       {
          rc = modbus_write_registers(ctx, 0, regCount, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
            for (i=0; i < rc; i++) {
                printf("reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
               }
           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                               
//        printf("FLUSH context write: %i\n",modbus_flush(ctx));
        fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
     modbus_close(ctx);
     modbus_free(ctx);
             fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
return fl;
}

int test_read_write(int devId1,int devId2, int regCount, int regNum, int bit){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
     uint16_t tab_reg2[1];
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
        response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
                                                       
       modbus_set_slave(ctx, devId1);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, regCount, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
            for (i=0; i < rc; i++) {
                printf("reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
               }
           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
     usleep(80 * 1000);
     
     //-------------------------------------------- ID 2
     if (fl==1)
     {
     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     fprintf(stderr, "%s\n", modbus_strerror(errno));       
       modbus_set_slave(ctx, devId2);
       connected = modbus_connect(ctx);
     
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
              tab_reg2[0]=1;
//              rc = modbus_write_registers(ctx, 0, regCount, tab_reg2);
              rc = modbus_write_registers(ctx, 0, 1, tab_reg2);                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
     
     printf("write end \n");
     }
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     
return fl;
}

int rl_write(int bit){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
     uint16_t tab_reg2[1];
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
        response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
                                                       
       modbus_set_slave(ctx,34);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
//       if(connected == 0)
//       printf("connected\n");

                                  
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
     
     
     //-------------------------------------------- ID 2
     
     


     //  if(connected == -1)
      // printf("Connection faile d\n");
      // if(connected == 0)
      // printf("connected\n");
              tab_reg2[0]=bit;

              rc = modbus_write_registers(ctx, 0, 1, tab_reg2);                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
     usleep(30 * 1000);
     modbus_flush(ctx);
//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       
     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     
return fl;
}

int signal_event_kb(char sName[100]){
int n=0;
int result=0;
  for (n=0;n<MAX_Signals;n++)
  {
   if (Signal_Array[n].Value[1] > 0)
      {
//        strcpy(sName,Signal_Array[n].Name);
//        result=Signal_Array[n].Value[1];
        printf ("[sName: %s ] [Value: %i ]",Signal_Array[n].Name,Signal_Array[n].Value[1]);
           if (strstr(Signal_Array[n].Name,"kei") == NULL) // search only button key pressed event
              {
                rl_write(1);
              }
      } 
  }
 return result; //if zerro then not found signals else number
}


//read central keyboard big panel Mb slave 31
int read_central_kb(){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
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
        response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
                                                       
       modbus_set_slave(ctx, 31);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 6, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("Central_KeyBoard reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
                //Buttons on keyboard
                //reg 0 brightness
                //reg 1 button led
                
                //reg 2
                get_reg_bit("kb.key.stop_oil_pump");
                signal_update("kb.key.stop_oil_pump",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit));
                //------------------------------------
                signal_update("kb.key.start_check",CheckBit(tab_reg[2],1));  
                signal_update("kb.key.start_oil_pump",CheckBit(tab_reg[2],2));  
                signal_update("kb.key.stop_check",CheckBit(tab_reg[2],3));  
                signal_update("kb.key.start_reloader",CheckBit(tab_reg[2],4));
                signal_update("kb.key.start_conveyor",CheckBit(tab_reg[2],5));
                signal_update("kb.key.stop_conveyor",CheckBit(tab_reg[2],6));
                signal_update("kb.key.stop_reloader",CheckBit(tab_reg[2],7));                
                signal_update("kb.key.start_stars",CheckBit(tab_reg[2],8));
                signal_update("kb.key.start_oil_station",CheckBit(tab_reg[2],9));
                signal_update("kb.key.start_hydratation",CheckBit(tab_reg[2],10));
                signal_update("kb.key.stop_stars",CheckBit(tab_reg[2],11));
                signal_update("kb.key.start_exec_dev",CheckBit(tab_reg[2],12));
                signal_update("kb.key.stop_oil_station",CheckBit(tab_reg[2],13));
                signal_update("kb.key.stop_hydratation",CheckBit(tab_reg[2],14));
                signal_update("kb.key.stop_exec_dev",CheckBit(tab_reg[2],15));
                //reg 3
                //extended joysticks and switches on keyboard
                signal_update("kb.kei1.mode2",CheckBit(tab_reg[3],0));
                signal_update("kb.kei1.mode1",CheckBit(tab_reg[3],1));
                signal_update("kb.kei1.post_conveyor",CheckBit(tab_reg[3],2));
                signal_update("kb.kei1.control2",CheckBit(tab_reg[3],3));
                signal_update("kb.kei1.control1",CheckBit(tab_reg[3],4));
                signal_update("kb.kei1.stop_alarm",CheckBit(tab_reg[3],5));
                signal_update("kb.kei1.sound_alarm",CheckBit(tab_reg[3],6));
                signal_update("kb.kei1.conveyor_left",CheckBit(tab_reg[3],7));
                signal_update("kb.kei1.conveyor_up",CheckBit(tab_reg[3],8));
                signal_update("kb.kei1.stop_all",CheckBit(tab_reg[3],9));
                signal_update("kb.kei1.start_all",CheckBit(tab_reg[3],10));
                signal_update("kb.kei1.power",CheckBit(tab_reg[3],11));
                signal_update("kb.kei1.conveyor_down",CheckBit(tab_reg[3],12));
                signal_update("kb.kei1.sourcer_down",CheckBit(tab_reg[3],13));
                signal_update("kb.kei1.sourcer_up",CheckBit(tab_reg[3],14));
                signal_update("kb.kei1.combain_support_down",CheckBit(tab_reg[3],15));
                
                //reg 4
                signal_update("kb.kei2.combain_support_up",CheckBit(tab_reg[4],0));
                signal_update("kb.kei2.conveyor_right",CheckBit(tab_reg[4],1));
                signal_update("kb.kei2.telescope_up",CheckBit(tab_reg[4],2));
                signal_update("kb.kei2.reserve5",CheckBit(tab_reg[4],3));
                signal_update("kb.kei2.reserve4",CheckBit(tab_reg[4],4));
                signal_update("kb.kei2.reserve3",CheckBit(tab_reg[4],5));
                signal_update("kb.kei2.reserve2",CheckBit(tab_reg[4],6));
                signal_update("kb.kei2.reserve1",CheckBit(tab_reg[4],7));
                signal_update("kb.kei2.left_truck_back",CheckBit(tab_reg[4],8));
                signal_update("kb.kei2.left_truck_forward",CheckBit(tab_reg[4],9));
                signal_update("kb.kei2.right_truck_back",CheckBit(tab_reg[4],10));
                signal_update("kb.kei2.right_truck_forward",CheckBit(tab_reg[4],11));
                signal_update("kb.kei2.acceleration",CheckBit(tab_reg[4],12));
                signal_update("kb.kei2.exec_dev_left",CheckBit(tab_reg[4],13));
                signal_update("kb.kei2.exec_dev_down",CheckBit(tab_reg[4],14));
                signal_update("kb.kei2.exec_dev_right",CheckBit(tab_reg[4],15));
                
                //reg 5
                signal_update("kb.kei3.exec_dev_up",CheckBit(tab_reg[5],0));
                signal_update("kb.kei3.reserve_down",CheckBit(tab_reg[5],1));
                signal_update("kb.kei3.reserve_up",CheckBit(tab_reg[5],2));
                signal_update("kb.kei3.telescope_down",CheckBit(tab_reg[5],3));
                
                
//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
     usleep(30 * 1000); //delay for Mod bus restore functional
     
     //-------------------------------------------- ID 2

//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     modbus_flush(ctx);
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       

     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             

return 0;
}


//read ADC1 Mb slave 35
int read_adc1(){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
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
        response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
                                                       
       modbus_set_slave(ctx, 35);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 8, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("ADC1 reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }


//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
     usleep(20 * 1000); //delay for Mod bus restore functional
     
     //-------------------------------------------- ID 2

//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     modbus_flush(ctx);
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       

     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     
return 0;
}


//read ADC2 Mb slave 36
int read_adc2(){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
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
        response_timeout.tv_sec = 1;
        response_timeout.tv_usec = 80;
        byte_timeout.tv_sec = 1;
        byte_timeout.tv_usec = 300;
        modbus_set_byte_timeout (ctx, &byte_timeout);
        modbus_set_response_timeout(ctx, &response_timeout);
        //modbus_set_byte_timeout(ctx, struct timeval *timeout);
        //modbus_rtu_set_rts_delay(ctx,40);
                                                       
       modbus_set_slave(ctx, 36);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection faile d\n");
       if(connected == 0)
       printf("connected\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 8, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "%s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("ADC2 reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }


//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
     usleep(20 * 1000); //delay for Mod bus restore functional
     
     //-------------------------------------------- ID 2

//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     modbus_flush(ctx);
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       

     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     
return 0;
}


int main ()
{
char Sig_Event[100];
// time_start();
 init_list();
 file_load();
 list(); //fill signals and echo to screen
// printf("Time: %ld ms\n", time_stop());
// time_start();
// printf ( "Found signals: %i\n\n",signal_update("kb.kei3.telescope_down",100));
// list();
// printf("Time: %ld ms\n", time_stop());
 //printf ("Test oil_pump_stop: %i\n",test_read (31,3,2,0));
 
//==================
pthread_t threads[NUM_THREADS];
    int status;
    int status_addr;
    int i;
    someArgs_t args[NUM_THREADS];
    const char *messages[] = {
    "First",
    NULL,
    "Third Message",
    "Fourth Message"
    };
         
         for (i = 0; i < NUM_THREADS; i++) {
              args[i].id = i;
             // args[i].msg = messages[i];
              args[i].SA_ptr = Signal_Array;
              args[i].counters=i;
             }
          for (i = 0; i < NUM_THREADS; i++) {
              status = pthread_create(&threads[i], NULL, helloWorld, (void*) &args[i]);
                  if (status != 0) {
                      printf("main error: can't create thread, status = %d\n", status);
                      exit(ERROR_CREATE_THREAD);
                      }
              }
         
printf("Main Message\n");
 
     for (i = 0; i < NUM_THREADS; i++) {
         status = pthread_join(threads[i], (void**)&status_addr);
          if (status != SUCCESS) {
              printf("main error: can't join thread, status = %d\n", status);
              exit(ERROR_JOIN_THREAD);
             }
          printf("joined with address %d\n", status_addr);
                                                                 }
                                                                  
          for (i = 0; i < NUM_THREADS; i++) {
               printf("thread %d arg.out = %d\n", i, args[i].out);
              }
       
//==================
 
/* 
 while(1)
 {
  time_start();
   //if ( test_read (31,3,2,0))  test_write (34,1,0,0);
    //  test_read (35,7,2,0);
//      test_read_write(31,34,3,2,0);
   read_central_kb();
   signal_event_kb(Sig_Event);
//   list();
// printf("Signal event name: %s\n",Sig_Event);
   read_adc1();
   read_adc2();
//  printf("Stop_oil_pump: %i\n",    signal_read("kb.key.stop_oil_pump"));  
  printf("Time: %ld ms\n", time_stop());
  }
 */
 list();
return 0;
}
