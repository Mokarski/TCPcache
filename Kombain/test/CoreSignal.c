#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // Порт, к которому подключается клиент

#define MAXDATASIZE 20 // максимальное число байт, принимаемых за один раз

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "/home/opc/Kombain/test/include/modbus/modbus.h"

#define MAX_Signals 350
#define STR_LEN_TXT 100
#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS        0
#define NUM_THREADS 2
#define Error_Report 0  //debug errors On=1 
#define VirtDev 30      //numbers of virtual modbus devices
#define VirtDevRegs 50
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

struct timeval tvr1,tvr2,dtvr;
struct timezone tzr;

void timer_start() {
gettimeofday(&tvr1,&tzr);
}

long timer_stop(){
gettimeofday(&tvr2,&tzr);
dtvr.tv_sec= tvr2.tv_sec -tvr1.tv_sec;
dtvr.tv_usec=tvr2.tv_usec-tvr1.tv_usec;
   if(dtvr.tv_usec<0) 
   { 
     dtvr.tv_sec--; dtvr.tv_usec+=1000000; 
   }
 return dtvr.tv_sec*1000+dtvr.tv_usec/1000;
}

//-----------------------------------------------------------------------
// array of SIgnals = Array of Events  and Indexes is match!!!!!!!!!
//-----------------------------------------------------------------------

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
  int ExState;        // 0-not executed/unknown 1-for execution,2-executed,3-expired,4-dropped  Thread Arbitrage
  int Ex_HW;   //Execution Hardware mark PcProc =1 Panel 43=2 Panel 10=3 | soft_class 
  int Ex_SF;   //Execution Software mark ModBusMaster_RTU =1 Modbus_Master_TCP=2 CoreSignal=100
  int Prio;    //Priority of signal 0 - no priority
  uint TypeTrigger; //signal has trigger state on/off
};
struct Signal Signal_Array[MAX_Signals];

//   for cache read all devices registers //
struct Mb_Device {
char Name[100];
int  MB_Id;                //Modbus device ID
int  MB_Registers[VirtDevRegs];     //Mb register number
 int Off;               //Signal not used if OFF = 1;
 uint EventTrigger;          //signal has trigger state 
 int For_Remove;        // mar for remove from tasks
} Mb_Device_Cache;

struct Mb_Device Device_Array [VirtDev]; //number of virtual devices

struct Mb_Event {
char Name[100];
int Value;
char Val_Type[10];    //type of value int or bit 
int Delay; //signal timer
int Start; //signal state start
int Stop;  //signal state stop
int Prio;    //Priority of signal 0 - no priority
 int  MB_Id;            //Modbus device ID
 int  MB_Reg_Num;       //Mb register number
 int  Bit_Pos;          //bit position
 int Off;               //Signal not used if OFF = 1;
 uint EventTrigger;          //signal has trigger state 
 int For_Remove;        // mar for remove from tasks
} Mb_event_action;

struct Mb_Event Event_Array [MAX_Signals]; // set numbers of threads = to numbers of tasks in Event_Array



typedef struct Discrete_Signals { // store one  signal state
  char Input_Signal[100];        //searched signal name
  char Reaction_Signal[100];     //reaction signal name
  int adc_val;                   //control this state on ADC
  int result;                    // returned value
  int Delay;                     //for delay action in ms
  int Start;
  int Stop;
  int Prio;           //Priority of signal 0 - no priority  
  int For_Remove;        // mar for remove from tasks
  
  uint Trigger;       //signal has trigger state
  struct Signal *SA_ptr; //pointer to signals array
  struct Mb_Event *MbEv_ptr; //pointer to event array
} Discrete_Signals_t;

 Discrete_Signals_t args[MAX_Signals]; // Create array of Tasks to control signals by threads
 

/*
typedef struct Discrete_Timer_Signals { // store one  signal state
  char *Input_Signal;    //searched signal name
  char *Reaction_Signal;    //reaction signal name
  int result;            // returned value
  int delay;
  struct Signal *SA_ptr; //pointer to signals array
  struct Mb_Event *MbEv_ptr; //pointer to event array
} Discrete_Tmr_Signals_t;
*/



//buffer for adresation and get Type of value
struct Reg { // for update signal function 
int Mb_Reg;
int Mb_Bit;
int Value;
char Val_Type[10];    //type of value int or bit 
} Reg_Bit;

/*
typedef struct someArgs_tag {
  int id;
  const char *msg;
  int out;
  int counters;
  struct Signal *SA_ptr;
} someArgs_t;
*/
int NumThreads; // for dinamic change numbers of trhreads
int MB_EVENTS_COUNTER;

void debug_print(int Num, char *Msg, char *InputChar, int InputInt){
if (Num == 1) printf("\nDEBUG:[ %s ] [ %s ] [ %i ]\n",Msg,InputChar,InputInt);
//return 0;
}

void add_task_mb(char sName[100], int start, int stop, int delay) //direct adding task in task list
{
  int n=0;
   for (n=0; n < MAX_Signals; n++){   
           if (strstr(Signal_Array[n].Name ,sName ) != NULL )
              {
                        strcpy( Event_Array[n].Name ,Signal_Array[n].Name); //copy signal params from Signal_Array to Mb_Evet_Array to mb_event_list
                        Event_Array[n].MB_Id = Signal_Array[n].MB_Id;
                        Event_Array[n].MB_Reg_Num = Signal_Array[n].MB_Reg_Num;
                        Event_Array[n].Bit_Pos = Signal_Array[n].Bit_Pos;
                        Event_Array[n].Value = Signal_Array[n].Value[1]; // Value array of int [0;1]    Val[0] for phys val, Val[1] for raw data             
                        Event_Array[n].Prio = Signal_Array[n].Prio;
                        strcpy( Event_Array[n].Val_Type,Signal_Array[n].Val_Type);
                        Event_Array[n].Delay = delay;                         
                        Event_Array[n].Start = start;
                        Event_Array[n].Stop = stop;
                        //Event_Array[n].ExState = ExStat;                       
              }        
     }
}

void* DS_thread(void *args) { //keyboard discrete signals thread function Sveto Klav
int n=0;
Discrete_Signals_t *arg = (Discrete_Signals_t*) args;

//printf(">>>Thread_Report: NumThreads %i\n",NumThreads);

//      printf("PTR SA: %s\n",arg->SA_ptr[arg->Input_Signal].Name);
   for (n=0; n < MAX_Signals; n++){   
   
//   for (n=0; n < NumThreads-1; n++){      
//      printf("THREAD Input Signal:[ %s ] Cyrrent_NAme:[ %s ]\n\n", arg->Input_Signal, arg->SA_ptr[n].Name);
        //if (arg->SA_ptr[n].Name == arg->Input_Signal )
        if (strstr(arg->SA_ptr[n].Name ,arg->Input_Signal ) != NULL )
        {
//              printf("THREAD Found Signal Name:[ %s ] IN Signal_Array\n\n", arg->Input_Signal);
              debug_print(Error_Report,"THREAD Found Signal Name: IN Signal_Array" ,arg->Input_Signal ,0);
           //arg->result=arg->SA_ptr[n].Value[1];           
//           if (arg->SA_ptr[n].Off == 1)
//           {
//            printf("Signal is OFF!!!\n");
//            break;
//           }
           
           if (arg->SA_ptr[n].ExState > 1) //allready executed or dropped
           {
            //printf("Signal is Allready Executed!!!\n");
            break;
           }
           
           if (arg->SA_ptr[n].ExState < 2) //
           {
//            printf("Signal[ %s  ] Execution state marked as Executed!!!\n",arg->SA_ptr[n].Name);
           debug_print(Error_Report,"SignalExecution state marked as Executed!!!" ,arg->SA_ptr[n].Name ,0);            
            arg->SA_ptr[n].ExState = 2; //state Executed
           }
           
           
           //Section for Int type signal <<<<<<<<<<<< --- INT
             if ( strstr(arg->SA_ptr[n].Val_Type ,"int" ) != NULL ){
           
             if (arg->SA_ptr[n].Value[1]==arg->adc_val) //check ADC measured is eq controlled
             {
                    int ind;
            // printf("THREAD Signal :[ %s ] = 1 \n\n", arg->Input_Signal);
              for (ind=0; ind < MAX_Signals; ind++)
                {
                   if (strstr( arg->SA_ptr[ind].Name,  arg->Reaction_Signal ) != NULL )                   
                   {
                      //printf(">>>THREAD Found Reaction_Signal INT :[ %s ]\n\n", arg->Reaction_Signal);
                      debug_print(Error_Report,">>THREAD Found Reaction_Signal INT :[ %s ]" ,arg->Reaction_Signal ,0);            
                        
                        //Attention bug with index!!!
                        //MB_EVENTS_COUNTER++;
                        // char Val_Type[10];    //type of value int or bit 
                        strcpy(arg->MbEv_ptr[ ind].Name ,arg->SA_ptr[ind].Name); //copy signal params from Signal_Array to Mb_Evet_Array
                        arg->MbEv_ptr[ ind].MB_Id = arg->SA_ptr[ind].MB_Id;
                        arg->MbEv_ptr[ ind].MB_Reg_Num = arg->SA_ptr[ind].MB_Reg_Num;
                        arg->MbEv_ptr[ ind].Bit_Pos = arg->SA_ptr[ind].Bit_Pos;
                        arg->MbEv_ptr[ind].Value = arg->SA_ptr[ind].Value[1]; // Value array of int [0;1]    Val[0] for phys val, Val[1] for raw data             
                        arg->MbEv_ptr[ind].Prio = arg->Prio;
                        arg->SA_ptr[ind].Prio = arg->Prio;   
                        
                     
                        //arg->MbEv_ptr[ind].Trigger = arg->Trigger;
                        //arg->SA_ptr[ind].Trigger = arg->Trigger;  
                        
                        strcpy( arg->MbEv_ptr[ ind].Val_Type, arg->SA_ptr[ind].Val_Type ); 
                                               
                           if (arg->SA_ptr[n].Off == 0) //signal not blocked
		               {
//                                   arg->MbEv_ptr[ind].Trigger = 1;
//                                   arg->SA_ptr[ind].Trigger = 1;  
                                      //printf("Signal is Active!!!\n");
                                      debug_print(Error_Report,"Signal is Active" ," " ,0);            
                                          if ( arg->Start == 1 ) arg->MbEv_ptr[ ind].Start=1;  //Action start
                                         //else arg->MbEv_ptr[ind].Start=0;                        
                        
                                          if ( arg->Stop == 1 ) arg->MbEv_ptr[ ind].Stop=1; //Action stop
                                          //else arg->MbEv_ptr[ind].Stop=0;
                        
                                
                                }
                                 
                              if (arg->SA_ptr[n].Off == 1) //signal blocked
	  	                 {
                                   //printf("Signal is Blocked!!!\n");                                
                                   debug_print(Error_Report,"Signal is Blocked OFF=1" ," " ,0);            
                                   break;
                                 }
                                
//                        if ( arg->Start == 1 ) arg->MbEv_ptr[ ind].Start=1;  //Action start
//                        //else arg->MbEv_ptr[ind].Start=0;                        
                        
//                        if ( arg->Stop == 1 ) arg->MbEv_ptr[ ind].Stop=1; //Action stop
//                        //else arg->MbEv_ptr[ind].Stop=0;
                        
                    /*    if ( arg->Delay > 1 ) 
                        {
                           //how to control legth of stack in time_wait_period? Because stack of task is changed!
                           arg->MbEv_ptr[ind].Delay=arg->Delay; //Timer Action 
                           arg->MbEv_ptr[ind].Start=1;  //Action start
                           //usleep( arg->Delay * 1000); // wait in ms
                           arg->MbEv_ptr[ind].Stop=1; //Action stop
                        }
                        */
                        
                    break;
                   }
                } 
             }
            }
            // END INT SECTION <<<<<<<<< ------ INT
           
           
           //Section For Bit_type signal <<<<<<<<<<< --- BIT
           if ( strstr(arg->SA_ptr[n].Val_Type ,"bit" ) != NULL ){
           
           if (arg->SA_ptr[n].Value[1] == 0) // if signal is bit type and is state zerro                       
           {
            // This Section works only when used trigger state !!!!!!!!
            // SIGNAL Type Trigger - while active is - 1 else 0  
            // other signal has two input state signals start and stop and depends on two logic - 1 (signal one and signal two)
	    //   WARNING !!!! KOSTILLO!!!
	    //   Value [2]
	    //   Value 1 - new signal state
	    //   Value 0 - previus signal state             
	    //
	    //             printf("Recived signal state: %i\n",arg->SA_ptr[n].Value[1]);
               debug_print(Error_Report,"Recived signal state:" ," " ,arg->SA_ptr[n].Value[1]);            
               
/*               if ( arg->SA_ptr[n].Value[0] == 1  ) {//previus state value 1 changed to zerro
                    arg->SA_ptr[n].Trigger=0; //trigger state is off
                    printf("Previus state - 1, new state - 0");
                   }
*/
               
              int ind;             
              for (ind=0; ind < MAX_Signals; ind++)
                {
                   if (strstr( arg->MbEv_ptr[ind].Name,  arg->Reaction_Signal ) != NULL )                   
                   {
                     //arg->MbEv_ptr[ind].Trigger = 0;
                      //printf("THREAD Found Reaction_Signal BIT:[ %s ] - 0\n\n", arg->Reaction_Signal);
                       debug_print(Error_Report,"THREAD Found Reaction_Signal BIT state:" ,arg->Reaction_Signal ,0);            
                       
                       if (arg->SA_ptr[ind].TypeTrigger == 1 ) {  // check if signal trigger type
                             if (arg->MbEv_ptr[ind].EventTrigger == 1){    //read previus state trigger from mB event list if signal changed from 1 -> 0
                                 printf("Trigger Reaction_Signal BIT 1 - > 0 :[ %s ] - 1\n\n", arg->Reaction_Signal);
                                 arg->MbEv_ptr[ind].Start=0;
                                 arg->MbEv_ptr[ind].Stop=1;            //Action trigger stop                                                                                             
                                 }   
                        
                             if (arg->MbEv_ptr[ind].EventTrigger == 0){    //read previus state trigger from mB event list  if no changes 0 -> 0 stop signal
                                 printf("Trigger Reaction_Signal BIT 0 - > 0 :[ %s ] - 1\n\n", arg->Reaction_Signal);
                                 arg->MbEv_ptr[ind].Start=0;
                                 arg->MbEv_ptr[ind].Stop=0;            //Action trigger stop                                                                 
                                 arg->MbEv_ptr[ind].For_Remove = 1;    //task marked as "for remove"
                                 }   
                      }                            
                   }
                }
            
           }
           
           if (arg->SA_ptr[n].Value[1]==1) // if signal is bit type and is active 
           { 
             int ind;
            // printf("THREAD Signal :[ %s ] = 1 \n\n", arg->Input_Signal);
              for (ind=0; ind < MAX_Signals; ind++)
                {
                   if (strstr( arg->SA_ptr[ind].Name,  arg->Reaction_Signal ) != NULL )                   
                   {
//                      printf(">>>THREAD Found Reaction_Signal BIT:[ %s ] - 1\n\n", arg->Reaction_Signal);
                        debug_print(Error_Report,"THREAD Found Reaction_Signal BIT state:" ,arg->Reaction_Signal ,1);                                    
                        
                        //Attention bug with index!!!
                        //
                        //copy signal params from Signal_Array to Mb_Evet_Array
                        
                        strcpy(arg->MbEv_ptr[ ind].Name ,arg->SA_ptr[ind].Name);
                        arg->MbEv_ptr[ ind].MB_Id = arg->SA_ptr[ind].MB_Id;
                        arg->MbEv_ptr[ ind].MB_Reg_Num = arg->SA_ptr[ind].MB_Reg_Num;
                        arg->MbEv_ptr[ ind].Bit_Pos = arg->SA_ptr[ind].Bit_Pos;
                        arg->MbEv_ptr[ind].Value = arg->SA_ptr[ind].Value[1];  //!!!! warning in Signal array Value has a array [0,1] used Value [1]
                        arg->MbEv_ptr[ind].Prio = arg->Prio;
                        arg->SA_ptr[ind].Prio = arg->Prio;   
                        
                        if ( arg->Trigger == 1) { // if parametr INPUT_SIGNAL set as  trigger then set trigger flag
                            arg->MbEv_ptr[ind].EventTrigger = 1; //mark trigger in event list
                            arg->SA_ptr[ind].TypeTrigger = 1;   //mark trigger in signal list
                            }
                        
                        strcpy( arg->MbEv_ptr[ ind].Val_Type, arg->SA_ptr[ind].Val_Type ); 
                                               
                       if (arg->SA_ptr[n].Off == 0) //signal not blocked
		          {
		            // arg->MbEv_ptr[ind].Trigger = arg->Trigger;
                            // arg->SA_ptr[ind].Trigger = arg->Trigger;  
                            // printf("Signal is Active!!!\n");
                            
                                      
                              if (  arg->MbEv_ptr[ind].EventTrigger == 1 ) {    // if signal type Trigger is set to 1
                                    arg->MbEv_ptr[ind].Start=1;                // Action trigger start                                                                 
                                  }
                        
                                         
                              if ( arg->Start == 1 ) arg->MbEv_ptr[ ind].Start=1;  //Action start
                                    
                              if ( arg->Stop == 1 ) arg->MbEv_ptr[ ind].Stop=1; //Action stop
                    
                                
                          }
                                
                           if (arg->SA_ptr[n].Off == 1) //signal blocked
		               {
                                      printf("Signal is Blocked!!!\n");                                
                                      debug_print(Error_Report,"Signal blosked: " ,arg->SA_ptr[n].Name ,0);                                                  
                                       break;
                                }
                                
//                        if ( arg->Start == 1 ) arg->MbEv_ptr[ ind].Start=1;  //Action start
//                        //else arg->MbEv_ptr[ind].Start=0;                        
                        
//                        if ( arg->Stop == 1 ) arg->MbEv_ptr[ ind].Stop=1; //Action stop
//                        //else arg->MbEv_ptr[ind].Stop=0;
                        
/*                        if ( arg->Delay > 1 ) 
                        {
                           //how to control legth of stack in time_wait_period? Because stack of task is changed!
                           arg->MbEv_ptr[ ind].Delay= arg->Delay; //Timer Action 
                           arg->MbEv_ptr[ ind].Start=1;  //Action start
                           usleep( arg->Delay * 1000); // wait in ms
                           arg->MbEv_ptr[ind].Stop=1; //Action stop
                        }
*/

                    break;
                   }
                }
            }  // <<< End Bit Section ------------     
            }
            break;
          }

       }
 return SUCCESS;
}

/*
void* Signal_Tmr_thread(void *args) { //timer for delay discrete signal
int n=0;
KB_Discrete_Signals_t *arg = (Discrete_Tmr_Signals_t*) args;
    
      printf("Signal_Tmr: %s\n", arg->Input_Signal);
//      printf("PTR SA: %s\n",arg->SA_ptr[arg->Input_Signal].Name);
   for (n=0; n < MAX_Signals; n++){
       if (arg->SA_ptr[n].Name == arg->Input_Signal){
           usleep( arg->delay * 1000); // wait in ms
           //arg->result=arg->SA_ptr[n].Value[1];
             SA_ptr[n].Value[1]="1"; // set value to 1
           break;
          }
       }
       
 return SUCCESS;
}
*/


void init_signal_list(void)
{
 register int t;
 for(t=0; t<MAX_Signals; ++t) Signal_Array[t].Name[0] = '\0';
}

void init_event_list(void)
{
 register int t;
 for(t=0; t<MAX_Signals; ++t) Event_Array[t].Name[0] = '\0';
}

void init_task_list(void)
{
 register int t;
 for(t=0; t<MAX_Signals; ++t) args[t].Input_Signal[0] = '\0';
}

void block_all_signals(void)
{
 register int t;
 for(t=0; t<MAX_Signals; ++t) 
  {
//   Signal_Array[t].Name[0] = '\0';
   Signal_Array[t].Off = 1; //signal marked as off
  }
}

void unblock_all_signals(void)
{
 register int t;
 for(t=0; t<MAX_Signals; ++t) 
  {
//   Signal_Array[t].Name[0] = '\0';
   Signal_Array[t].Off = 0; //signal marked as off
  }
}

void signal_list (void) //print all signals
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

void device_list (void) //print all signals
{
register int t;

for (t=0; t<30; ++t) {
if(Device_Array[t].Name[0]) 
  {
   printf("Signal Name: %s\n", Device_Array[t].Name);   
   printf("MB ID: %i\n", Device_Array[t].MB_Id);
   int z=0;
     for(z=0; z<VirtDevRegs; z++) { //50 numbers of virtual registers in struct Device_Array
      printf ("Register[%i] : [ %i ]\n",z, Device_Array[t].MB_Registers[z]);
     }

  }
}
printf("\n\n");
}

void signal_state_list (void) //print all none ZERRO signals
{
register int t;
printf("----------------------Signals State List--------------------------\n");
for (t=0; t<MAX_Signals; ++t) {
if(Signal_Array[t].Name[0]) 
  {
   if (Signal_Array[t].Value[1]>0){
      printf("Signal Name: %s\n", Signal_Array[t].Name);
      printf ("Values: [Hi: %i] [Low: %i]\n\n", Signal_Array[t].Value[0],Signal_Array[t].Value[1]);
   }
  }
}
printf("----------------------END Signal State List-------------------------\n\n");
}


void event_list (void)
{
register int t;
printf("----------------------[ ModBus TASK List: ]-------------------------\n\n");
for (t=0; t<MAX_Signals; ++t) {
if(Event_Array[t].Name[0]) 
  {
   printf("Signal Name: %s\n", Event_Array[t].Name);
   printf("MB ID: %i\n", Event_Array[t].MB_Id);
   printf ("MB Register Num: %i\n", Event_Array[t].MB_Reg_Num);
   printf ("MB Register BitNum: %i\n", Event_Array[t].Bit_Pos);
   printf ("Val type: %s\n", Event_Array[t].Val_Type);   
//   printf ("Value: %s\n", Event_Array[t].Val_Type);   
     printf ("Signal OFF: %i\n", Event_Array[t].Off);
     printf ("Signal Start: %i\n", Event_Array[t].Start);
     printf ("Signal Stop: %i\n",  Event_Array[t].Stop);     
   printf ("Value: [ %i ] \n\n",  Event_Array[t].Value);
  }
}
printf("----------------------END TASK List-------------------------\n\n");
printf("\n\n");
}

int num_event_list (void) //numbers of tasks in event list
{
  register int t;
  int cnt=0;
 for (t=0; t<MAX_Signals; ++t) {
     if(Event_Array[t].Name[0]) 
       {
         cnt++;

       }
     }
printf("number of input modbus events %i \n\n",cnt);
return cnt;
}

int num_task_list (void) //numbers of tasks in event list
{
  register int t;
  int cnt=0;
  printf("---------------ARGS ARRAY TASK  LIST-----------------\n");
 for (t=0; t<MAX_Signals; ++t) {
     if(args[t].Input_Signal[0]) 
       {
       //DEBUG all info 
       /*
          printf ("Input_signal %s\n", args[t].Input_Signal);        //searched signal name
          printf ("Reaction_signal %s\n", args[t].Reaction_Signal);  //reaction signal name
          printf ("Start %i\n", args[t].Start);                 
          printf ("Stop %i\n", args[t].Stop);                   
								    //  int result;                    // returned value
								    //  int Delay;                     //for delay action in ms
								    //  int Start;
								    //  int Stop;
								    //  int Prio;           //Priority of signal 0 - no priority  
								    //  uint Trigger;       //signal has trigger state
          printf ("Trigger %i\n", args[t].Trigger);                     
          */
          debug_print(Error_Report,"args Input Signal: " ,args[t].Input_Signal ,0);                                                  
          debug_print(Error_Report,"args Input Signal: " ,args[t].Reaction_Signal ,0);
          debug_print(Error_Report,"Start: " ,"" , args[t].Start);
          debug_print(Error_Report,"Stop: " ,"" , args[t].Stop);          
          debug_print(Error_Report,"Trigger: " ,"" , args[t].Trigger);          
         cnt++;

       }
    }
 printf ("-----------------------------------------\n");                   
printf("number of input tasks %i \n\n",cnt);
return cnt;
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

int event_update_start_stop (char sName[100],int Start, int Stop){ //function to update parameters in tcp_cache_server list
int n=0;
int cnt=0;
  for (n=0;n<MAX_Signals;n++)
  {
   if (strstr(Event_Array[n].Name,sName) != NULL) 
      {
//        Event_Array[n].Value[1]=sVal;
        Event_Array[n].Start=Start;
        Event_Array[n].Stop=Stop;        
        cnt++;
        break;
      } 
  }
 return cnt; //if zerro then not found signals else number of matches
}


int signal_update_ex (char sName[100],int sVal, int Execution_State){ //function to update parameters in signals array
int n=0;
int cnt=0;
  for (n=0; n < MAX_Signals; n++)
  {
   if (strstr(Signal_Array[n].Name,sName) != NULL) 
      {
        Signal_Array[n].Value[1]=sVal;
        Signal_Array[n].ExState=Execution_State;
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
        //int Value;
        //char Val_Type[10];    //type of value int or bit 
        Reg_Bit.Mb_Reg=Signal_Array[n].MB_Reg_Num;
        Reg_Bit.Mb_Bit=Signal_Array[n].Bit_Pos;
        Reg_Bit.Value=Signal_Array[n].Value[1];
        strcpy(Reg_Bit.Val_Type,Signal_Array[n].Val_Type);
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

int mb_write(int devId, int regCount, int regNum, int bit, int val, int mb_R){
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
       printf("MB_Write: Connection failed %i\n", devId);
       if(connected == 0)
       printf("MB_Write: connected %i\n", devId);
       int cn=0;
       if ( val == 0 )
       {
         if (bit == 0)    tab_reg[0]=0xfffe & mb_R;       
         if (bit == 1)    tab_reg[0]=0xfffd & mb_R;
         if (bit == 2)    tab_reg[0]=0xfffb & mb_R ;
         if (bit == 3)    tab_reg[0]=0xfff7 & mb_R ;       
         if (bit == 4)    tab_reg[0]=0xffef & mb_R ;       
         if (bit == 5)    tab_reg[0]=0xffdf & mb_R ;       
         if (bit == 6)    tab_reg[0]=0xffbf & mb_R ;              
         if (bit == 7)    tab_reg[0]=0xff7f & mb_R ;      
         if (bit == 8)    tab_reg[0]=0xfeff & mb_R ;
         if (bit == 9)    tab_reg[0]=0xfdff & mb_R ;            
         if (bit == 10)    tab_reg[0]=0xfbff & mb_R ;                     
         if (bit == 11)    tab_reg[0]=0xf7ff & mb_R ;                     
         if (bit == 12)    tab_reg[0]=0xefff & mb_R ;                              
         if (bit == 13)    tab_reg[0]=0xdfff & mb_R ;                              
         if (bit == 14)    tab_reg[0]=0xbfff & mb_R ;                              
         if (bit == 15)    tab_reg[0]=0x7fff & mb_R ;         
         
         printf(">>Mb_write ZERRRO <<\n");
       }
       
       if (val) // register 16 bit
       {
         if (bit == 0)    tab_reg[0]=0x1 | mb_R;       
         if (bit == 1)    tab_reg[0]=0x2 | mb_R;
         if (bit == 2)    tab_reg[0]=0x4 | mb_R ;
         if (bit == 3)    tab_reg[0]=0x8 | mb_R ;       
         if (bit == 4)    tab_reg[0]=0x10| mb_R ;       
         if (bit == 5)    tab_reg[0]=0x20 | mb_R ;       
         if (bit == 6)    tab_reg[0]=0x40 | mb_R ;              
         if (bit == 7)    tab_reg[0]=0x80 | mb_R ;      
         if (bit == 8)    tab_reg[0]=0x100 | mb_R ;
         if (bit == 9)    tab_reg[0]=0x200 | mb_R ;            
         if (bit == 10)    tab_reg[0]=0x400 | mb_R ;                     
         if (bit == 11)    tab_reg[0]=0x800 | mb_R ;                     
         if (bit == 12)    tab_reg[0]=0x1000 | mb_R ;                              
         if (bit == 13)    tab_reg[0]=0x2000 | mb_R ;                              
         if (bit == 14)    tab_reg[0]=0x4000 | mb_R ;                              
         if (bit == 15)    tab_reg[0]=0x8000 | mb_R ;                              
        }        
        
//        if ( regCount == 0 ) regCount=1;
          regCount=1;
        
       printf ("[Dev_ID %i] [regCount %i] [regNum %i] [bitNum %i] [Val %i]",devId,regCount,regNum,bit,tab_reg[0]);
       while(cn!=1)
       {
          rc = modbus_write_registers(ctx, regNum, regCount, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "MB_ERROR: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                               
         usleep(30 * 1000);
        printf("FLUSH context write: %i\n",modbus_flush(ctx));
        fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
     modbus_close(ctx);
     modbus_free(ctx);
//     modbus_close(ctx);
     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
return fl;
}

int mb_write_virtdev(int devId,  int regNum, int bit, int val, int mb_R){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[8];
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
       printf("MB_Write: Connection failed %i\n", devId);
       if(connected == 0)
       printf("MB_Write: connected %i\n", devId);
       int cn=0;
       if ( val == 0 )
       {
         if (bit == 0)    tab_reg[0]=0xfffe & mb_R;       
         if (bit == 1)    tab_reg[0]=0xfffd & mb_R;
         if (bit == 2)    tab_reg[0]=0xfffb & mb_R ;
         if (bit == 3)    tab_reg[0]=0xfff7 & mb_R ;       
         if (bit == 4)    tab_reg[0]=0xffef & mb_R ;       
         if (bit == 5)    tab_reg[0]=0xffdf & mb_R ;       
         if (bit == 6)    tab_reg[0]=0xffbf & mb_R ;              
         if (bit == 7)    tab_reg[0]=0xff7f & mb_R ;      
         if (bit == 8)    tab_reg[0]=0xfeff & mb_R ;
         if (bit == 9)    tab_reg[0]=0xfdff & mb_R ;            
         if (bit == 10)    tab_reg[0]=0xfbff & mb_R ;                     
         if (bit == 11)    tab_reg[0]=0xf7ff & mb_R ;                     
         if (bit == 12)    tab_reg[0]=0xefff & mb_R ;                              
         if (bit == 13)    tab_reg[0]=0xdfff & mb_R ;                              
         if (bit == 14)    tab_reg[0]=0xbfff & mb_R ;                              
         if (bit == 15)    tab_reg[0]=0x7fff & mb_R ;         
         
         printf(">>Mb_write ZERRRO <<\n");
       }
       
       if (val) // register 16 bit
       {
         if (bit == 0)    tab_reg[0]=0x1 | mb_R;       
         if (bit == 1)    tab_reg[0]=0x2 | mb_R;
         if (bit == 2)    tab_reg[0]=0x4 | mb_R ;
         if (bit == 3)    tab_reg[0]=0x8 | mb_R ;       
         if (bit == 4)    tab_reg[0]=0x10| mb_R ;       
         if (bit == 5)    tab_reg[0]=0x20 | mb_R ;       
         if (bit == 6)    tab_reg[0]=0x40 | mb_R ;              
         if (bit == 7)    tab_reg[0]=0x80 | mb_R ;      
         if (bit == 8)    tab_reg[0]=0x100 | mb_R ;
         if (bit == 9)    tab_reg[0]=0x200 | mb_R ;            
         if (bit == 10)    tab_reg[0]=0x400 | mb_R ;                     
         if (bit == 11)    tab_reg[0]=0x800 | mb_R ;                     
         if (bit == 12)    tab_reg[0]=0x1000 | mb_R ;                              
         if (bit == 13)    tab_reg[0]=0x2000 | mb_R ;                              
         if (bit == 14)    tab_reg[0]=0x4000 | mb_R ;                              
         if (bit == 15)    tab_reg[0]=0x8000 | mb_R ;                              
        }        
        
//        if ( regCount == 0 ) regCount=1;
          //regCount=1;
        
       printf ("[Dev_ID %i] [regCount %i] [regNum %i] [bitNum %i] [Val %i]",devId,regNum,1,bit,tab_reg[0]);
       while(cn!=1)
       {
          rc = modbus_write_registers(ctx, regNum, 1, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "MB_ERROR: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                               
         usleep(30 * 1000);
        printf("FLUSH context write: %i\n",modbus_flush(ctx));
        fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
     modbus_close(ctx);
     modbus_free(ctx);
//     modbus_close(ctx);
     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
return fl;
}


int mb_write_int(int devId, int regNum, int val){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[1];
     int rc;
     int i;
     int fl;
      int regCount;
                      
       ctx = modbus_new_rtu("/dev/ttySP0", 115200, 'N', 8, 1);
                           
       if (ctx == NULL) {
           fprintf(stderr, "Unable to create the libmodbus context\n");
           return -1;
        }
                                                       
       modbus_set_slave(ctx, devId);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Mb_Write_Int: Connection faile %i\n",devId);
       if(connected == 0)
       printf("Mb_Write_Int: connected %i\n",devId);
       int cn=0;

//        if ( regCount == 0 ) regCount=1;
          regCount=1;
        
       printf ("[Dev_ID %i] [regCount %i] [regNum %i]  [Val %i ]",devId,regCount,regNum,val);
       tab_reg[0]=val;
       while(cn!=1)
       {
          
          rc = modbus_write_registers(ctx, regNum, regCount, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "MB_ERROR: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                               
         usleep(30 * 1000);
        printf("FLUSH context write: %i\n",modbus_flush(ctx));
        fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                          
     modbus_close(ctx);
     modbus_free(ctx);
//     modbus_close(ctx);
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

int signal_state_by_mask(char sName[100]){
int n=0;
int result=0;
printf ("-------------------------------state By MASK------------------------------------------- \n");
  for (n=0;n<MAX_Signals;n++)
  {
   if (Signal_Array[n].Value[1] > 0)
      {
//        strcpy(sName,Signal_Array[n].Name);
//        result=Signal_Array[n].Value[1];
//        printf ("[sName: %s ] [Value: %i ]",Signal_Array[n].Name,Signal_Array[n].Value[1]);
           if ( strstr(Signal_Array[n].Name, sName) != NULL) // search only button key pressed event
              {
                //rl_write(1);

                //printf ("[Name: %s ] [Value: %i ] \n",Signal_Array[n].Name,Signal_Array[n].Value[1]);
                
                debug_print(Error_Report,"Signal_Array" ,Signal_Array[n].Name ,Signal_Array[n].Value[1]);

              }
      } 
  }
printf ("-----------------------------END By MASK----------------------------------------- \n");                  
 return result; //if zerro then not found signals else number
}



int signal_by_mask(char sName[100]){
int n=0;
int result=0;
printf ("-------------------------------state By MASK------------------------------------------- \n");
  for (n=0;n<MAX_Signals;n++)
  {
//   if (Signal_Array[n].Value[1] > 0)
//      {
//        strcpy(sName,Signal_Array[n].Name);
//        result=Signal_Array[n].Value[1];
//        printf ("[sName: %s ] [Value: %i ]",Signal_Array[n].Name,Signal_Array[n].Value[1]);
           if ( strstr(Signal_Array[n].Name, sName) != NULL) // search only button key pressed event
              {
                //rl_write(1);

                printf ("[Name: %s ] [Value: %i ] \n",Signal_Array[n].Name,Signal_Array[n].Value[1]);

              }
    //  } 
  }
printf ("-----------------------------END By MASK----------------------------------------- \n");                  
 return result; //if zerro then not found signals else number
}


int signal_block_by_mask(char sName[100]){
int n=0;
int result=0;
printf ("-------------------------------Block By MASK------------------------------------------- \n");
  for (n=0;n<MAX_Signals;n++)
  {
//   if (Signal_Array[n].Value[1] > 0)
//      {
//        strcpy(sName,Signal_Array[n].Name);
//        result=Signal_Array[n].Value[1];
//        printf ("[sName: %s ] [Value: %i ]",Signal_Array[n].Name,Signal_Array[n].Value[1]);
           if ( strstr(Signal_Array[n].Name, sName) != NULL) // search only button key pressed event
              {
                

                printf ("[Name: %s ] [Value: %i ] \n",Signal_Array[n].Name,Signal_Array[n].Value[1]);
                Signal_Array[n].Off=1;

              }
//      } 
  }
 printf ("-----------------------------END Block By MASK----------------------------------------- \n");                
 return result; //if zerro then not found signals else number
}

int signal_unblock_by_mask(char sName[100]){
int n=0;
int result=0;
printf ("-------------------------------Unblock By MASK------------------------------------------- \n");
  for (n=0;n<MAX_Signals;n++)
  {
//   if (Signal_Array[n].Value[1] > 0)
//      {
//        strcpy(sName,Signal_Array[n].Name);
//        result=Signal_Array[n].Value[1];
//        printf ("[sName: %s ] [Value: %i ]",Signal_Array[n].Name,Signal_Array[n].Value[1]);
           if ( strstr(Signal_Array[n].Name, sName) != NULL) // search only button key pressed event
              {
                

                printf ("[Name: %s ] [Value: %i ] \n",Signal_Array[n].Name,Signal_Array[n].Value[1]);
                Signal_Array[n].Off=0;

              }
//      } 
  }
 printf ("-----------------------------END Unblock By MASK----------------------------------------- \n");                
 return result; //if zerro then not found signals else number
}


void mb_event_list (void)
{
register int t;
for (t=0; t<MAX_Signals; ++t) {
if(Event_Array[t].Name[0]) 
  {
   printf("------------ mb_event_list ---------------\n");
/*   printf("mb_event_write: \n");
   printf("Signal Name: %s\n", Event_Array[t].Name);
   printf("MB ID: %i\n", Event_Array[t].MB_Id);
   printf ("MB Register Num: %i\n", Event_Array[t].MB_Reg_Num);
   printf ("MB Register BitNum: %i\n", Event_Array[t].Bit_Pos);
      printf ("MB Start: %i\n", Event_Array[t].Start);
      printf ("MB Stop: %i\n", Event_Array[t].Stop);
       printf ("Prio: %i\n", Event_Array[t].Prio);
       printf ("Trigger: %i\n", Event_Array[t].EventTrigger);
       printf ("On/Off: %i\n", Event_Array[t].Off);
*/
//       printf ("ExState: %i\n", Event_Array[t].ExState);
   //int mb_write(int devId,int regCount, int regNum, int bit)

       if (Event_Array[t].Prio == 1)  // if we get priority signal execute this 
          {
          
              if (Event_Array[t].Stop == 1) 
                  {
                    printf(">>Priority Event STOP!!!<< \n");                    
                    // if ( strstr(arg->SA_ptr[n].Val_Type ,"int" ) != NULL )

                    if ( strstr(Event_Array[t].Val_Type,"bit") != NULL) { //for signal bit type                   
                         //int mb_r = mb_read(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num);
                         int mb_r =  mb_read_virtdev (Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num);
                         mb_write(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num, Event_Array[t].MB_Reg_Num , Event_Array[t].Bit_Pos,0, mb_r);
                        }
                    printf("Reset flags Start/Stop to Zerro.\n");
                    Event_Array[t].Start =0;
                    Event_Array[t].Stop =0;
                  }
         
              if (Event_Array[t].Start == 1)  
                  {
                    printf(">>Priority Event START!!!<< \n");                  
                
                    if ( strstr(Event_Array[t].Val_Type,"bit") != NULL) { //for signal bit type            
                         //int mb_r = mb_read(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num);
                         int mb_r =  mb_read_virtdev (Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num);
                         mb_write(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num, Event_Array[t].MB_Reg_Num , Event_Array[t].Bit_Pos,1, mb_r);
                        }
                  }
                  
    /*            if (Event_Array[t].Trigger == 1)  
                {
                      block_all_signals();
                      printf("--TOTAL SIGNALS BLOCK!!!--\n");
                }
                
                if (Event_Array[t].Trigger == 0)  
                {
                      unblock_all_signals();
                      printf("--TOTAL UNBLOCK!!!--\n");
                }
                */
           }
           
        //if we get no priority signals and state OFF is zerro
   if (Event_Array[t].Prio == 0) 
      {        
          if(Signal_Array[t].Off == 0) // if signal not deactivated then execute 
            {
              printf("Signal name in Signal_Array[ %s ]\n",Signal_Array[t].Name);
              printf("Signal name in Event_Array[ %s ]\n",Event_Array[t].Name);
              
              if (Event_Array[t].Start == 1)  
                 {
                   printf(">>Event START! [ %s ]<< \n",Event_Array[t].Name);
                   if ( strstr(Event_Array[t].Val_Type,"bit") != NULL) { //for signal bit type
                        int mb_r = mb_read(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num);                   
                        mb_write(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num, Event_Array[t].MB_Reg_Num , Event_Array[t].Bit_Pos,1,mb_r);
                       }
                       
                   if ( strstr(Event_Array[t].Val_Type ,"int" ) != NULL ) //for signal int type
                       mb_write_int(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num,  Event_Array[t].Value);                   
                 }
                 
              if (Event_Array[t].Stop == 1)               
                  {
                     printf(">>Event STOP!!! [ %s ]<< \n",Event_Array[t].Name);
                     if ( strstr(Event_Array[t].Val_Type,"bit") != NULL) { //for signal bit type
                          int mb_r = mb_read(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num);
                          mb_write(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num, Event_Array[t].MB_Reg_Num , Event_Array[t].Bit_Pos,0,mb_r);
                         }
                         
                   if ( strstr(Event_Array[t].Val_Type ,"int" ) != NULL ) //for signal int type
                       mb_write_int(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num,  0x0);  //write 0x0 - kostillo
                       
                     printf("Reset  Start/Stop to -> 0.\n");
                     Event_Array[t].Start =0;
                     Event_Array[t].Stop =0;
                  } 
                  
//                if (Event_Array[t].Trigger == 1)  
//                {
//                  mb_write(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num, Event_Array[t].MB_Reg_Num , Event_Array[t].Bit_Pos,1);                
//                }
                
//                if (Event_Array[t].Trigger == 0)  
//                {
//                  mb_write(Event_Array[t].MB_Id, Event_Array[t].MB_Reg_Num, Event_Array[t].MB_Reg_Num , Event_Array[t].Bit_Pos,0);     
//                }
            }
        }

   printf("\n ========================END mb_event_list================================= \n");

  }
}
printf("\n\n");
}


//read RPDU_wired mode keyboard  Mb slave 73
int read_wired_rpdu_kb(){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
     int rc;
     int i;
     int fl;
     printf ("Read wired RPDU \n");                      
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
                                                       
       modbus_set_slave(ctx, 73);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection failed RPDU wired\n");
       if(connected == 0)
       printf("connected RPDU wired\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 6, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "RPDU wired: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("Central_KeyBoard reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
                //Buttons on keyboard
                //reg 0 brightness
                //reg 1 button led
                
                get_reg_bit("rpdu485c.connect");
                signal_update_ex("rpdu485c.connect",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.oil_station_green");
                signal_update_ex("rpdu485c.kbl.oil_station_green",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.reloader_green");
                signal_update_ex("rpdu485c.kbl.reloader_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.conveyor_green");
                signal_update_ex("rpdu485c.kbl.conveyor_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.loader_green");
                signal_update_ex("rpdu485c.kbl.loader_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.loader_red");
                signal_update_ex("rpdu485c.kbl.loader_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.exec_dev_green");
                signal_update_ex("rpdu485c.kbl.exec_dev_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.support_green");
                signal_update_ex("rpdu485c.kbl.support_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.sourcer_green");
                signal_update_ex("rpdu485c.kbl.sourcer_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.reserve_green");
                signal_update_ex("rpdu485c.kbl.reserve_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.reserve_red");
                signal_update_ex("rpdu485c.kbl.reserve_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.acceleration_green");
                signal_update_ex("rpdu485c.kbl.acceleration_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.acceleration_red");
                signal_update_ex("rpdu485c.kbl.acceleration_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.telescope_green");
                signal_update_ex("rpdu485c.kbl.telescope_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kbl.alarm_red");
                signal_update_ex("rpdu485c.kbl.alarm_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.oil_station_up");
                signal_update_ex("rpdu485c.kei.oil_station_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.oil_station_down");
                signal_update_ex("rpdu485c.kei.oil_station_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.reloader_up");
                signal_update_ex("rpdu485c.kei.reloader_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.reloader_down");
                signal_update_ex("rpdu485c.kei.reloader_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.conveyor_up");
                signal_update_ex("rpdu485c.kei.conveyor_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.conveyor_down");
                signal_update_ex("rpdu485c.kei.conveyor_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.loader_up");
                signal_update_ex("rpdu485c.kei.loader_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.loader_down");
                signal_update_ex("rpdu485c.kei.loader_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.exec_dev_up");
                signal_update_ex("rpdu485c.kei.exec_dev_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.exec_dev_down");
                signal_update_ex("rpdu485c.kei.exec_dev_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.support_up");
                signal_update_ex("rpdu485c.kei.support_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.support_down");
                signal_update_ex("rpdu485c.kei.support_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.sourcer_up");
                signal_update_ex("rpdu485c.kei.sourcer_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.sourcer_down");
                signal_update_ex("rpdu485c.kei.sourcer_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.reserve_up");
                signal_update_ex("rpdu485c.kei.reserve_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.reserve_down");
                signal_update_ex("rpdu485c.kei.reserve_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.start_all" );
                signal_update_ex("rpdu485c.kei.start_all" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.stop_all");
                signal_update_ex("rpdu485c.kei.stop_all" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.sound_beepl");
                signal_update_ex("rpdu485c.sound_beepl" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.stop_loader");
                signal_update_ex("rpdu485c.kei.stop_loader" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.acceleration_up");
                signal_update_ex("rpdu485c.kei.acceleration_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.acceleration_down");
                signal_update_ex("rpdu485c.kei.acceleration_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.telescope_up");
                signal_update_ex("rpdu485c.kei.telescope_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.telescope_down");
                signal_update_ex("rpdu485c.kei.telescope_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_forward");
                signal_update_ex("rpdu485c.kei.joy_forward" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_back");
                signal_update_ex("rpdu485c.kei.joy_back" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_left");
                signal_update_ex("rpdu485c.kei.joy_left" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_right");
                signal_update_ex("rpdu485c.kei.joy_right" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_conv_up");
                signal_update_ex("rpdu485c.kei.joy_conv_up",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_conv_down");
                signal_update_ex("rpdu485c.kei.joy_conv_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_conv_left");
                signal_update_ex("rpdu485c.kei.joy_conv_left" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_conv_right");
                signal_update_ex("rpdu485c.kei.joy_conv_right" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_exec_dev_up");
                signal_update_ex("rpdu485c.kei.joy_exec_dev_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_exec_dev_down");
                signal_update_ex("rpdu485c.kei.joy_exec_dev_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_exec_dev_right");                
                signal_update_ex("rpdu485c.kei.joy_exec_dev_right" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.joy_exec_dev_left");
                signal_update_ex("rpdu485c.kei.joy_exec_dev_left" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485c.kei.crit_stop");
                signal_update_ex("rpdu485c.kei.crit_stop" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                
                

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



//read RPDU keyboard  Mb slave 33
int read_rpdu_kb(){
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[32];
     int rc;
     int i;
     int fl;
                      
    printf ("Read RPDU \n");
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
                                                       
       modbus_set_slave(ctx, 33);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection failed RPDU\n");
       if(connected == 0)
       printf("connected RPDU\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 6, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "RPDU: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("Central_KeyBoard reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
                //Buttons on keyboard
                //reg 0 brightness
                //reg 1 button led
                
                get_reg_bit("rpdu485.connect");
                signal_update_ex("rpdu485.connect",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.oil_station_green");
                signal_update_ex("rpdu485.kbl.oil_station_green",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.reloader_green");
                signal_update_ex("rpdu485.kbl.reloader_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.conveyor_green");
                signal_update_ex("rpdu485.kbl.conveyor_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.loader_green");
                signal_update_ex("rpdu485.kbl.loader_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.loader_red");
                signal_update_ex("rpdu485.kbl.loader_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.exec_dev_green");
                signal_update_ex("rpdu485.kbl.exec_dev_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.support_green");
                signal_update_ex("rpdu485.kbl.support_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.sourcer_green");
                signal_update_ex("rpdu485.kbl.sourcer_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.reserve_green");
                signal_update_ex("rpdu485.kbl.reserve_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.reserve_red");
                signal_update_ex("rpdu485.kbl.reserve_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.acceleration_green");
                signal_update_ex("rpdu485.kbl.acceleration_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.acceleration_red");
                signal_update_ex("rpdu485.kbl.acceleration_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.telescope_green");
                signal_update_ex("rpdu485.kbl.telescope_green" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kbl.alarm_red");
                signal_update_ex("rpdu485.kbl.alarm_red" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.oil_station_up");
                signal_update_ex("rpdu485.kei.oil_station_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.oil_station_down");
                signal_update_ex("rpdu485.kei.oil_station_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.reloader_up");
                signal_update_ex("rpdu485.kei.reloader_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.reloader_down");
                signal_update_ex("rpdu485.kei.reloader_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.conveyor_up");
                signal_update_ex("rpdu485.kei.conveyor_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.conveyor_down");
                signal_update_ex("rpdu485.kei.conveyor_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.loader_up");
                signal_update_ex("rpdu485.kei.loader_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.loader_down");
                signal_update_ex("rpdu485.kei.loader_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.exec_dev_up");
                signal_update_ex("rpdu485.kei.exec_dev_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.exec_dev_down");
                signal_update_ex("rpdu485.kei.exec_dev_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.support_up");
                signal_update_ex("rpdu485.kei.support_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.support_down");
                signal_update_ex("rpdu485.kei.support_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.sourcer_up");
                signal_update_ex("rpdu485.kei.sourcer_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.sourcer_down");
                signal_update_ex("rpdu485.kei.sourcer_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.reserve_up");
                signal_update_ex("rpdu485.kei.reserve_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.reserve_down");
                signal_update_ex("rpdu485.kei.reserve_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.start_all" );
                signal_update_ex("rpdu485.kei.start_all" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.stop_all");
                signal_update_ex("rpdu485.kei.stop_all" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.sound_beepl");
                signal_update_ex("rpdu485.sound_beepl" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.stop_loader");
                signal_update_ex("rpdu485.kei.stop_loader" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.acceleration_up");
                signal_update_ex("rpdu485.kei.acceleration_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.acceleration_down");
                signal_update_ex("rpdu485.kei.acceleration_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.telescope_up");
                signal_update_ex("rpdu485.kei.telescope_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.telescope_down");
                signal_update_ex("rpdu485.kei.telescope_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_forward");
                signal_update_ex("rpdu485.kei.joy_forward" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_back");
                signal_update_ex("rpdu485.kei.joy_back" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_left");
                signal_update_ex("rpdu485.kei.joy_left" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_right");
                signal_update_ex("rpdu485.kei.joy_right" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_conv_up");
                signal_update_ex("rpdu485.kei.joy_conv_up",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_conv_down");
                signal_update_ex("rpdu485.kei.joy_conv_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_conv_left");
                signal_update_ex("rpdu485.kei.joy_conv_left" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_conv_right");
                signal_update_ex("rpdu485.kei.joy_conv_right" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_exec_dev_up");
                signal_update_ex("rpdu485.kei.joy_exec_dev_up" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_exec_dev_down");
                signal_update_ex("rpdu485.kei.joy_exec_dev_down" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_exec_dev_right");                
                signal_update_ex("rpdu485.kei.joy_exec_dev_right" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.joy_exec_dev_left");
                signal_update_ex("rpdu485.kei.joy_exec_dev_left" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("rpdu485.kei.crit_stop");
                signal_update_ex("rpdu485.kei.crit_stop" ,CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                
                

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
     usleep(50 * 1000); //delay for Mod bus restore functional
     
     //-------------------------------------------- ID 2

//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     modbus_flush(ctx);
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       

     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             

return 0;
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
       printf("Connection failed CENTRAL_KB\n");
       if(connected == 0)
       printf("connected CENTRAL_KB\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 6, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "CENTRAL_KB: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                                            
//            for (i=0; i < rc; i++) {
//                printf("Central_KeyBoard reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }
                //Buttons on keyboard
                //reg 0 brightness
                //reg 1 button led
                
                
                //reg 1
                get_reg_bit("kb.kbl.stop_oil_pump");
                signal_update_ex("kb.kbl.stop_oil_pump",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("kb.kbl.start_check");
                signal_update_ex("kb.kbl.start_check",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                get_reg_bit("kb.kbl.start_oil_pump");
                signal_update_ex("kb.kbl.stop_oil_pump",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1);
                
                //--------------------------------------------------
                //reg 2
                //universal solution based on loaded signals cfg
                get_reg_bit("kb.key.stop_oil_pump");
                signal_update_ex("kb.key.stop_oil_pump",CheckBit(tab_reg[Reg_Bit.Mb_Reg],Reg_Bit.Mb_Bit), 1); // set ExState Flag to 1
                //------------------------------------
                // no universal solution
                
                signal_update_ex("kb.key.start_check",CheckBit(tab_reg[2],1), 1);  
                signal_update_ex("kb.key.start_oil_pump",CheckBit(tab_reg[2],2), 1);  
                signal_update_ex("kb.key.stop_check",CheckBit(tab_reg[2],3), 1);  
                signal_update_ex("kb.key.start_reloader",CheckBit(tab_reg[2],4), 1);
                signal_update_ex("kb.key.start_conveyor",CheckBit(tab_reg[2],5), 1);
                signal_update_ex("kb.key.stop_conveyor",CheckBit(tab_reg[2],6), 1);
                signal_update_ex("kb.key.stop_reloader",CheckBit(tab_reg[2],7), 1);                
                signal_update_ex("kb.key.start_stars",CheckBit(tab_reg[2],8), 1);
                
                signal_update_ex("kb.key.start_oil_station",  CheckBit(tab_reg[2],9), 1); //set ExState flag to 1
                signal_update_ex("kb.key.start_hydratation",CheckBit(tab_reg[2],10), 1);
                signal_update_ex("kb.key.stop_stars",CheckBit(tab_reg[2],11), 1);
                signal_update_ex("kb.key.start_exec_dev",CheckBit(tab_reg[2],12), 1);
                signal_update_ex("kb.key.stop_oil_station",CheckBit(tab_reg[2],13), 1);  //set ExState flag to 1
                signal_update_ex("kb.key.stop_hydratation",CheckBit(tab_reg[2],14), 1);
                signal_update_ex("kb.key.stop_exec_dev",CheckBit(tab_reg[2],15), 1);

                //reg 3
                //extended joysticks and switches on keyboard
                signal_update_ex("kb.kei1.mode2",CheckBit(tab_reg[3],0),1);
                signal_update_ex("kb.kei1.mode1",CheckBit(tab_reg[3],1),1);
                signal_update_ex("kb.kei1.post_conveyor",CheckBit(tab_reg[3],2),1);
                signal_update_ex("kb.kei1.control2",CheckBit(tab_reg[3],3), 1);
                signal_update_ex("kb.kei1.control1",CheckBit(tab_reg[3],4), 1);
                signal_update_ex("kb.kei1.stop_alarm",CheckBit(tab_reg[3],5), 1);
                signal_update_ex("kb.kei1.sound_alarm",CheckBit(tab_reg[3],6), 1);
                signal_update_ex("kb.kei1.conveyor_left",CheckBit(tab_reg[3],7), 1);
                signal_update_ex("kb.kei1.conveyor_up",CheckBit(tab_reg[3],8), 1);
                signal_update_ex("kb.kei1.stop_all",CheckBit(tab_reg[3],9), 1);
                signal_update_ex("kb.kei1.start_all",CheckBit(tab_reg[3],10), 1);
                signal_update_ex("kb.kei1.power",CheckBit(tab_reg[3],11), 1);
                signal_update_ex("kb.kei1.conveyor_down",CheckBit(tab_reg[3],12), 1);
                signal_update_ex("kb.kei1.sourcer_down",CheckBit(tab_reg[3],13), 1);
                signal_update_ex("kb.kei1.sourcer_up",CheckBit(tab_reg[3],14), 1);
                signal_update_ex("kb.kei1.combain_support_down",CheckBit(tab_reg[3],15), 1);
                
                //reg 4
                signal_update_ex("kb.kei2.combain_support_up",CheckBit(tab_reg[4],0), 1);
                signal_update_ex("kb.kei2.conveyor_right",CheckBit(tab_reg[4],1), 1);
                signal_update_ex("kb.kei2.telescope_up",CheckBit(tab_reg[4],2), 1);
                signal_update_ex("kb.kei2.reserve5",CheckBit(tab_reg[4],3), 1);
                signal_update_ex("kb.kei2.reserve4",CheckBit(tab_reg[4],4), 1);
                signal_update_ex("kb.kei2.reserve3",CheckBit(tab_reg[4],5), 1);
                signal_update_ex("kb.kei2.reserve2",CheckBit(tab_reg[4],6), 1);
                signal_update_ex("kb.kei2.reserve1",CheckBit(tab_reg[4],7), 1);
                signal_update_ex("kb.kei2.left_truck_back",CheckBit(tab_reg[4],8), 1);
                signal_update_ex("kb.kei2.left_truck_forward",CheckBit(tab_reg[4],9), 1);
                signal_update_ex("kb.kei2.right_truck_back",CheckBit(tab_reg[4],10), 1);
                signal_update_ex("kb.kei2.right_truck_forward",CheckBit(tab_reg[4],11), 1);
                signal_update_ex("kb.kei2.acceleration",CheckBit(tab_reg[4],12), 1);
                signal_update_ex("kb.kei2.exec_dev_left",CheckBit(tab_reg[4],13), 1);
                signal_update_ex("kb.kei2.exec_dev_down",CheckBit(tab_reg[4],14), 1);
                signal_update_ex("kb.kei2.exec_dev_right",CheckBit(tab_reg[4],15), 1);
                
                //reg 5
                signal_update_ex("kb.kei3.exec_dev_up",CheckBit(tab_reg[5],0), 1);
                signal_update_ex("kb.kei3.reserve_down",CheckBit(tab_reg[5],1), 1);
                signal_update_ex("kb.kei3.reserve_up",CheckBit(tab_reg[5],2), 1);
                signal_update_ex("kb.kei3.telescope_down",CheckBit(tab_reg[5],3), 1);
                
                
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
       printf("Connection failed ADC1\n");
       if(connected == 0)
       printf("connected ADC1\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 8, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "ADC1: %s\n", modbus_strerror(errno));
              return -1;
            } 
                                                                                                            
                //reg 1
                get_reg_bit("ad1.adc1_raw_value"); // fill buffer array
                signal_update_ex("ad1.adc1_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc1_phys_value"); // fill buffer array
                signal_update_ex("ad1.adc1_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc2_raw_value"); // fill buffer array
                signal_update_ex("ad1.adc2_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc2_phys_value"); // fill buffer array
                signal_update_ex("ad1.adc2_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc3_raw_value"); // fill buffer array
                signal_update_ex("ad1.adc3_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc3_phys_value"); // fill buffer array
                signal_update_ex("ad1.adc3_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc4_raw_value"); // fill buffer array
                signal_update_ex("ad1.adc4_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad1.adc4_phys_value"); // fill buffer array
                signal_update_ex("ad1.adc4_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                                                                                                                            


          cn++;
        }         
        //debug
                for (i=0; i < 8; i++) 
                {
//                  printf("ADC1 reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
                  debug_print(Error_Report," ADC1: tab_reg  \n" ," " ,tab_reg[i]);
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
       printf("Connection failed ADC2\n");
       if(connected == 0)
       printf("connected ADC2\n");
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, 8, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "ADC2: %s\n", modbus_strerror(errno));
              return -1;
            } 
            
                //reg 1
                get_reg_bit("ad2.adc1_raw_value"); // fill buffer array
                signal_update_ex("ad2.adc1_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc1_phys_value"); // fill buffer array
                signal_update_ex("ad2.adc1_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc2_raw_value"); // fill buffer array
                signal_update_ex("ad2.adc2_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc2_phys_value"); // fill buffer array
                signal_update_ex("ad2.adc2_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc3_raw_value"); // fill buffer array
                signal_update_ex("ad2.adc3_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc3_phys_value"); // fill buffer array
                signal_update_ex("ad2.adc3_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc4_raw_value"); // fill buffer array
                signal_update_ex("ad2.adc4_raw_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                
                get_reg_bit("ad2.adc4_phys_value"); // fill buffer array
                signal_update_ex("ad2.adc4_phys_value",tab_reg[Reg_Bit.Mb_Reg], 1);
                                                                                                                            


//           fl=(CheckBit(tab_reg[regNum],bit));

         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
    
    //debug
            for (i=0; i < 8; i++) {
                //printf("ADC2 reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
                debug_print(Error_Report," ADC2: tab_reg  \n" ," " ,tab_reg[i]);
               }

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


/*
//for cache read all devices registers
struct Mb_Device {
char Name[100];
int  MB_Id;                //Modbus device ID
int  MB_Registers[50];     //Mb register number
 int Off;               //Signal not used if OFF = 1;
 uint EventTrigger;          //signal has trigger state 
 int For_Remove;        // mar for remove from tasks
} Mb_Device_Cache;

struct Mb_Device Device_Array [30]; //number of virtual devices
*/


int mb_read_dCache(int dIndex,char Name[100], int ID, int reg_count){ //read from real devices to cache
     int connected;
     modbus_t *ctx;
     uint16_t tab_reg[50];
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
                                                       
       modbus_set_slave(ctx, ID);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection failed %i\n",ID);
       if(connected == 0)
       printf("connected %i\n",ID);
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, 0, reg_count, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "MB_READ: %s\n", modbus_strerror(errno));
              return -1;
            } 
            
         cn++;
        }           
    strcpy(Device_Array[dIndex].Name,Name);
    Device_Array[dIndex].MB_Id=ID;                                 //Modbus device ID
    int cx=0;
     for (cx=0; cx < reg_count; cx++){
        Device_Array[dIndex].MB_Registers[cx]=tab_reg[cx];     //Mb register number
       }
        
                                         
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
    
    //debug
//            for (i=0; i < 8; i++) {
//                printf("ADC2 reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }

     usleep(20 * 1000); //delay for Mod bus restore functional
     
     //-------------------------------------------- ID 2

//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     modbus_flush(ctx);
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       

     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     
return tab_reg[0];
}




//read one register
int mb_read_virtdev(int ID, int reg_num){
int result;
int z=0;
  for (z=0; z < VirtDev; z++)
  {
     if( Device_Array[z].MB_Id == ID ) {   //search Modbus device by ID
        result = Device_Array[z].MB_Registers[reg_num];

      }
  }


return result;
}

int mb_read(int ID, int reg_num){
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
                                                       
       modbus_set_slave(ctx, ID);
       connected = modbus_connect(ctx);
                                                                
       if(connected == -1)
       printf("Connection failed %i\n",ID);
       if(connected == 0)
       printf("connected %i\n",ID);
       int cn=0;
       while(cn!=1)
       {
          rc = modbus_read_registers(ctx, reg_num, 1, tab_reg);
                                                                                               
          if (rc == -1) {
              fprintf(stderr, "MB_READ: %s\n", modbus_strerror(errno));
              return -1;
            } 
            
         cn++;
        }                                            
     //printf("FLUSH context read: %i\n",modbus_flush(ctx));
     //fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
//     modbus_close(ctx);
    
    //debug
//            for (i=0; i < 8; i++) {
//                printf("ADC2 reg[%i]=%i (0x%X)\n", i, tab_reg[i], tab_reg[i]);            
//               }

     usleep(20 * 1000); //delay for Mod bus restore functional
     
     //-------------------------------------------- ID 2

//     printf("FLUSH context read: %i\n",modbus_flush(ctx));
     modbus_flush(ctx);
//     fprintf(stderr, "%s\n", modbus_strerror(errno));       

     
     modbus_close(ctx);
     modbus_free(ctx); //close COM ?
//     fprintf(stderr, "%s\n", modbus_strerror(errno));                                                                                             
     
return tab_reg[0];
}


void Core_Logic (void) // MOZGG - logical core of all
{
register int t;
  for (t=0; t<MAX_Signals; ++t) {
     if(Signal_Array[t].Name[0]) 
       {
        
       }
    }
}

int prog_sound(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task SOUND \n");          
      //preset signal config
     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.sound_value"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     
     
     /*
     n=n+1; //increment stack index
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     */

return n; //return number in TASK stack
}



int prog_rpdu_sound(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RPDU SOUND \n");          
      //preset signal config
     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.sound_beepl"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.sound_value"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     
return n; //return number in TASK stack
}

int prog_led(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task LED \n");          
      //preset signal config
     signal_update("rsrs2.sound_value",1028); //siren1,siren2 -on, and leds is on 
     printf ("Sound_LED int signal state: %i\n", signal_read("rsrs2.sound_value"));

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.sound_value"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     
     
     /*
     n=n+1; //increment stack index
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     */

return n; //return number in TASK stack
}


int prog_support(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task SUPPORT \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.combain_support_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on9"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.combain_support_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on8"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     /*
     n=n+1; //increment stack index
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     */

return n; //return number in TASK stack
}

int prog_rpdu_support(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RPDU SUPPORT \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.support_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on8"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.support_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on9"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     /*
     n=n+1; //increment stack index
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     */

return n; //return number in TASK stack
}


int prog_left_truck(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task LEFT TRUCK \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.left_truck_back"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on10"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.left_truck_forward"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on11"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     /*
     n=n+1; //increment stack index
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     */

return n; //return number in TASK stack
}

int prog_right_truck(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RIGHT TRUCK \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.right_truck_back"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on0"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.right_truck_forward"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on1"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     /*
     n=n+1; //increment stack index
     strcpy(args[n].Input_Signal, "kb.kei1.sound_alarm"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     */

return n; //return number in TASK stack
}

int prog_conv_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task CONVEYOR JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.conveyor_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on2"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.conveyor_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on3"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.conveyor_left"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on4"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.conveyor_right"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on5"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      

return n; //return number in TASK stack
}

int prog_rpdu_conv_joy(int n) {
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RPDU CONVEYOR JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.joy_conv_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on2"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.joy_conv_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on3"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.joy_conv_left"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on4"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.joy_conv_right"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on5"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      

return n; //return number in TASK stack
}


int prog_acceleration(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task ACCELERATION \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.acceleration"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on0"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

/*     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.right_truck_forward"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on1"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
*/

return n; //return number in TASK stack
}


int prog_rpdu_acceleration(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task ACCELERATION \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.acceleration_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on0"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

/*     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.right_truck_forward"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on1"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array      
     
*/

return n; //return number in TASK stack
}

int prog_telescope_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task TELESCOPE JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.telescope_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on4"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei3.telescope_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on5"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

return n; //return number in TASK stack
}


int prog_rpdu_telescope_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RPDU TELESCOPE JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
//rpdu485.kei.telescope_up     
     strcpy(args[n].Input_Signal, "rpdu485.kei.telescope_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on4"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.telescope_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on5"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

return n; //return number in TASK stack
}

int prog_rpdu_sourcer_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RPDU SOURCER JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.sourcer_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on8"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.sourcer_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on9"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

return n; //return number in TASK stack
}


int prog_sourcer_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task SOURCER JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.sourcer_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on8"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei1.sourcer_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on9"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

return n; //return number in TASK stack
}

int prog_exec_dev_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task EXEC DEV JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.exec_dev_left"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.exec_dev_right"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on3"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     
     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei3.exec_dev_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on6"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 


     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "kb.kei2.exec_dev_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on7"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 


return n; //return number in TASK stack
}


int prog_rpdu_exec_dev_joy(int n){
// kb.kei1.sound_alarm as trigger signal
      printf ("Task RPDU EXEC DEV JOY \n");          
      //preset signal config
//     signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
//     printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));
//     printf("TASK SUPPORT\n");

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.joy_exec_dev_left"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 

     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.joy_exec_dev_right"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on3"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 
     
     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.exec_dev_up"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on6"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 


     n=n+1; //increment stack index
     
     strcpy(args[n].Input_Signal, "rpdu485.kei.exec_dev_down"); // Searched name
     //rsrs2.state_sound1_led
     //strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u1_on7"); // Reaction name
     args[n].Trigger=1; //action trigger
     args[n].SA_ptr = Signal_Array;  //init ptr to signals array
     args[n].MbEv_ptr = Event_Array; //init ptr to events array 


return n; //return number in TASK stack
}


int prog_rpdu_start_all ( int N) {
 N=N+1;
 strcpy(args[N].Input_Signal, "rpdu485.kei.start_all"); // Searched name
     strcpy(args[N].Reaction_Signal, "rl.relay2"); // Reaction name
       args[N].Start=1; //action 
       args[N].SA_ptr = Signal_Array;  //init ptr to signals array
       args[N].MbEv_ptr = Event_Array; //init ptr to events array                  
 N=N+1;
     strcpy(args[N].Input_Signal, "rpdu485.kei.stop_all"); // Searched name
     strcpy(args[N].Reaction_Signal, "rl.relay2"); // Reaction name
       args[N].Stop=1; //action 
       args[N].SA_ptr = Signal_Array;  //init ptr to signals array
       args[N].MbEv_ptr = Event_Array; //init ptr to events array 
return N;
}

int prog_test ()
{

     //---------------- test task list ---------------------
     printf ("Tasks creation \n");     
     strcpy(args[0].Input_Signal, "kb.key.start_oil_station"); // Searched name
     strcpy(args[0].Reaction_Signal, "rl.relay2"); // Reaction name
      args[0].Start=1; //action 
      args[0].SA_ptr = Signal_Array;  //init ptr to signals array
      args[0].MbEv_ptr = Event_Array; //init ptr to events array

     strcpy(args[1].Input_Signal, "kb.key.stop_oil_station"); // Searched name
     strcpy(args[1].Reaction_Signal, "rl.relay2"); // Reaction name
       args[1].Stop=1; //action 
       args[1].SA_ptr = Signal_Array;  //init ptr to signals array
       args[1].MbEv_ptr = Event_Array; //init ptr to events array
       
    strcpy(args[2].Input_Signal, "kb.kei1.stop_all"); // Searched name
     strcpy(args[2].Reaction_Signal, "rl.relay2"); // Reaction name
       args[2].Stop=1; //action 
       args[2].SA_ptr = Signal_Array;  //init ptr to signals array
       args[2].MbEv_ptr = Event_Array; //init ptr to events array
    
     strcpy(args[3].Input_Signal, "kb.kei1.start_all"); // Searched name
     strcpy(args[3].Reaction_Signal, "rl.relay2"); // Reaction name
       args[3].Start=1; //action 
       args[3].SA_ptr = Signal_Array;  //init ptr to signals array
       args[3].MbEv_ptr = Event_Array; //init ptr to events array    
        
     strcpy(args[4].Input_Signal, "kb.key.start_oil_pump"); // Searched name
     strcpy(args[4].Reaction_Signal, "rsrs.rm_u2_on0"); // Reaction name
       args[4].Start=1; //action 
       args[4].SA_ptr = Signal_Array;  //init ptr to signals array
       args[4].MbEv_ptr = Event_Array; //init ptr to events array    
        
     strcpy(args[5].Input_Signal, "kb.key.stop_oil_pump"); // Searched name
     strcpy(args[5].Reaction_Signal, "rsrs.rm_u2_on0"); // Reaction name
       args[5].Stop=1; //action 
       args[5].SA_ptr = Signal_Array;  //init ptr to signals array
       args[5].MbEv_ptr = Event_Array; //init ptr to events array            
        
        
     strcpy(args[6].Input_Signal, "rpdu485.kei.start_all"); // Searched name
     strcpy(args[6].Reaction_Signal, "rl.relay2"); // Reaction name
       args[6].Start=1; //action 
       args[6].SA_ptr = Signal_Array;  //init ptr to signals array
       args[6].MbEv_ptr = Event_Array; //init ptr to events array                  

     strcpy(args[7].Input_Signal, "rpdu485.kei.stop_all"); // Searched name
     strcpy(args[7].Reaction_Signal, "rl.relay2"); // Reaction name
       args[7].Stop=1; //action 
       args[7].SA_ptr = Signal_Array;  //init ptr to signals array
       args[7].MbEv_ptr = Event_Array; //init ptr to events array 
       
//       rsrs2.state_sound1_led                
        
     strcpy(args[8].Input_Signal, "kb.key.start_conveyor"); // Searched name
     strcpy(args[8].Reaction_Signal, "rsrs.rm_u1_on0"); // Reaction name
       args[8].Start=1; //action 
       args[8].SA_ptr = Signal_Array;  //init ptr to signals array
       args[8].MbEv_ptr = Event_Array; //init ptr to events array                  

     strcpy(args[9].Input_Signal, "kb.key.stop_conveyor"); // Searched name
     strcpy(args[9].Reaction_Signal, "rsrs.rm_u1_on0"); // Reaction name
       args[9].Stop=1; //action 
       args[9].SA_ptr = Signal_Array;  //init ptr to signals array
       args[9].MbEv_ptr = Event_Array; //init ptr to events array 

     strcpy(args[10].Input_Signal, "kb.key.start_hydratation"); // Searched name
     strcpy(args[10].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
       args[10].Start=1; //action 
       args[10].SA_ptr = Signal_Array;  //init ptr to signals array
       args[10].MbEv_ptr = Event_Array; //init ptr to events array                  

     strcpy(args[11].Input_Signal, "kb.key.stop_hydratation"); // Searched name
     strcpy(args[11].Reaction_Signal, "rsrs2.state_sound1_led"); // Reaction name
       args[11].Stop=1; //action 
       args[11].SA_ptr = Signal_Array;  //init ptr to signals array
       args[11].MbEv_ptr = Event_Array; //init ptr to events array 
//kb.kei2.telescope_up
     strcpy(args[12].Input_Signal, "kb.kei2.telescope_up"); // Searched name
     strcpy(args[12].Reaction_Signal, "rsrs.rm_u1_on1"); // Reaction name
       args[12].Start=1; //action 
       args[12].SA_ptr = Signal_Array;  //init ptr to signals array
       args[12].MbEv_ptr = Event_Array; //init ptr to events array                  

     strcpy(args[13].Input_Signal, "kb.kei3.telescope_down"); // Searched name
     strcpy(args[13].Reaction_Signal, "rsrs.rm_u1_on1"); // Reaction name
       args[13].Stop=1; //action 
       args[13].SA_ptr = Signal_Array;  //init ptr to signals array
       args[13].MbEv_ptr = Event_Array; //init ptr to events array 

// kb.kei1.sound_alarm as trigger signal
    strcpy(args[14].Input_Signal, " kb.kei1.sound_alarm"); // Searched name
     strcpy(args[14].Reaction_Signal, "rsrs.rm_u1_on2"); // Reaction name
     args[14].Trigger=1; //action 
       args[14].SA_ptr = Signal_Array;  //init ptr to signals array
       args[14].MbEv_ptr = Event_Array; //init ptr to events array 
       
/*     strcpy(args[2].Input_Signal, "kb.kei1.stop_alarm"); // STOP GRIBOK
     strcpy(args[2].Reaction_Signal, "rl.relay2"); // Reaction name
       args[2].Stop=1; //action 
       args[2].Prio=1; //priority action 
       args[2].SA_ptr = Signal_Array;  //init ptr to signals array
       args[2].MbEv_ptr = Event_Array; //init ptr to events array
*/    
    
    /*
     strcpy(args[2].Input_Signal, "ad2.adc4_raw_value"); // STOP GRIBOK TEST !!!!!!!!
     strcpy(args[2].Reaction_Signal, "rl.relay1"); // Reaction name
       args[2].Stop=1; //action 
//       args[2].Prio=1; //priority action 
       args[2].adc_val = 29;// 2416 - no alarm, 29 - alarm, check this value on adc
       args[2].SA_ptr = Signal_Array;  //init ptr to signals array
       args[2].MbEv_ptr = Event_Array; //init ptr to events array    
      */
       printf("ADC2_ad4:[ %i ]\n",signal_read("ad2.adc4_raw_value"));
    

   

       
     printf ("Tasks assigned \n");          
     //-----------------------------------------------------
return 0;
}

int prog_gribok()
{
  int porog_adc=2400; //constant gribok 24xx
  int gribok=0;
  int result =0;
  int gribok_state_trigger=0;
 
printf("-- GRIBOK test --");
         gribok_state_trigger = signal_read("prog.alarm_gribok");
         gribok = signal_read("ad2.adc4_raw_value");
         printf (" gribok_state_trigger [%i]\n",  gribok_state_trigger);
        if ( gribok  < porog_adc ) //gribok is Actived
          {
           //old value 24xx
           //new value 2163 bad!!!
           block_all_signals();
           
           printf("Block All by GRIBOK - [ %i ] < 24xx!\n", gribok);            
           signal_update("prog.alarm_gribok",1);
           
           //what to do if Gribok active
            event_update_start_stop("rl.relay2",1,0);
            mb_event_list(); // write to modbus
            //init_signal_list(); //erase all signals
            init_event_list();  //erase all events            
            result = 2; //error
          } 
        
        if ( gribok > porog_adc  && signal_read("prog.alarm_gribok") == 1 ) //unblock ALL 
	  {
            unblock_all_signals();
            signal_update("prog.alarm_gribok",0); //reset flag alarm its my NEW SIGNAL!!!
            printf("UNBLOCK ALL SiGNALS by GRIBOK!\n");
            //event_update_start_stop("rl.relay2",0,0); // if start=0 and stop=0 signal not executed by modbus
            result = 1;
          }
          
return result;
}

int prog_hydratation(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task HYDRATATION \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_hydratation"); // Searched name
       strcpy(args[n].Reaction_Signal, "rl.relay1"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_hydratation");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);            
            add_task_mb("kb.kbl.start_hydratation",1,0,0);
//            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
//            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_hydratation"); // Searched name
     strcpy(args[n].Reaction_Signal, "rl.relay1"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_hydratation");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_hydratation",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_oil_station(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task OILSTATION \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_oil_station"); // Searched name
       strcpy(args[n].Reaction_Signal, "rl.relay2"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_oil_station");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("kb.kbl.start_oil_station",1,0,0);
            
    //        add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
    //        add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_oil_station"); // Searched name
     strcpy(args[n].Reaction_Signal, "rl.relay2"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.start_oil_station");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_oil_station",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}


int prog_rpdu_oil_station(int n) //switch signal
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task RPDU OILSTATION \n");          
      
    //  signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
    //  printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "rpdu485.kei.oil_station_up"); // Searched name
       strcpy(args[n].Reaction_Signal, "rl.relay2"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("rpdu485.kei.oil_station_up");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

//            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
//            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("rpdu485.kbl.oil_station_green",1,0,0);
            
//            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
//            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "rpdu485.kei.oil_station_down"); // Searched name
     strcpy(args[n].Reaction_Signal, "rl.relay2"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
    
        signal_on2 = signal_read("rpdu485.kei.oil_station_down");   

        if (signal_on2 == 1){
//            add_task_mb("rpdu485.kei.oil_station_up",0,1,0); //led on
            add_task_mb("rpdu485.kbl.oil_station_green",0,1,0);
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_rpdu_reloader_start(int n) //switch signal
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task RPDU RELOADER \n");          
      
    //  signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
    //  printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "rpdu485.kei.reloader_up"); // Searched name
       strcpy(args[n].Reaction_Signal, "rl.relay2"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("rpdu485.kei.reloader_up");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

//            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
//            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("rpdu485.kbl.reloader_green",1,0,0);
            
//            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
//            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "rpdu485.kei.reloader_down"); // Searched name
     strcpy(args[n].Reaction_Signal, "rl.relay2"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("rpdu485.kbl.reloader_down");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
//            add_task_mb("rpdu485.kei.oil_station_up",0,1,0); //led on
            add_task_mb("rpdu485.kbl.reloader_green",0,1,0);
          result =0;
          }
      
return n; //return numbers in TASK stack
}



int prog_reloader(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task RELOADER \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_reloader"); // Searched name
       strcpy(args[n].Reaction_Signal, "rl.relay3"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_reloader");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("kb.kbl.start_reloader",1,0,0);
            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_reloader"); // Searched name
     strcpy(args[n].Reaction_Signal, "rl.relay3"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_reloader");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_reloader",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_rpdu_reloader(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task RPDU RELOADER \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "rpdu485.kei.reloader_up"); // Searched name
       strcpy(args[n].Reaction_Signal, "none"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_reloader");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("kb.kbl.start_reloader",1,0,0);
            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "rpdu485.kei.reloader_down"); // Searched name
     strcpy(args[n].Reaction_Signal, "none"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_reloader");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_reloader",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_conveyor(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task CONVEYOR \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_conveyor"); // Searched name
       strcpy(args[n].Reaction_Signal, "rl.relay4"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_conveyor");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("kb.kbl.start_conveyor",1,0,0);
            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_conveyor"); // Searched name
     strcpy(args[n].Reaction_Signal, "rl.relay4"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_conveyor");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_conveyor",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_stars(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task STARS \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_stars"); // Searched name
       strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on6"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_stars");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("kb.kbl.start_stars",1,0,0);
            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_stars"); // Searched name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on6"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_stars");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_stars",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_exec_dev(int n)
{
int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task EXEC_DEV \n");          
      
      signal_update("rsrs2.sound_value",1285); //siren1,siren2 -on, and leds is on 
      printf ("Sound int signal state: %i\n", signal_read("rsrs2.sound_value"));


      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_exec_dev"); // Searched name
       strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on7"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_exec_dev");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast

            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);                        
            add_task_mb("kb.kbl.start_exec_dev",1,0,0);
            add_task_mb( "rsrs2.sound_value",1,0,0);
//            usleep(600*1000);
            add_task_mb( "rsrs2.sound_value",0,1,0);
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_exec_dev"); // Searched name
     strcpy(args[n].Reaction_Signal, "rsrs.rm_u2_on7"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_exec_dev");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_exec_dev",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_oil_pump (int n)
{

int result=0;
int signal_on1=0;
int signal_on2=0;
     //---------------- task list ---------------------
//     kb.kbl.led_contrast
//     signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 


      printf ("Task OIL PUMP \n");          
      n=n+1;
       strcpy(args[n].Input_Signal, "kb.key.start_oil_pump"); // Searched name
       strcpy(args[n].Reaction_Signal, "none"); // Reaction name
       args[n].Start=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array 

       signal_on1=signal_read("kb.key.start_oil_pump");   
       
       if (signal_on1 == 1){ //on button press button oil pump start
    //      init_event_list(); //clear all events task
    //      signal_block_by_mask("kb.key"); //block alljoystick and buttons from keyboard
    //      signal_block_by_mask("kb.kei"); //block alljoystick and buttons from keyboard     
    //      signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
    //      signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
    //      signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
    //      signal_unblock_by_mask("kb.key.start_oil_pump"); 
    //     kb.kbl.led_contrast
            signal_update("kb.kbl.led_contrast",85); //Keyboard contrast 
            add_task_mb("kb.kbl.led_contrast",1,0,0);
            add_task_mb("kb.kbl.start_oil_pump",1,0,0); //led on
          result=1;
          
          
          }
              
   
     n=n+1;        
     strcpy(args[n].Input_Signal, "kb.key.stop_oil_pump"); // Searched name
     strcpy(args[n].Reaction_Signal, "none"); // Reaction name
       args[n].Stop=1; //action 
       args[n].SA_ptr = Signal_Array;  //init ptr to signals array
       args[n].MbEv_ptr = Event_Array; //init ptr to events array           
        signal_on2=signal_read("kb.key.stop_oil_pump");   

        if (signal_on2 == 1){
          signal_unblock_by_mask("kb.key"); //unblock 
          signal_unblock_by_mask("kb.kei"); //unblock      
//          signal_unblock_by_mask("kb.kei1.mode");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.post");     //unblock toggles
//          signal_unblock_by_mask("kb.kei1.control");  //unblock toggles     
//          signal_unblock_by_mask("kb.key.start_oil_pump");    //unblock oil pump start/stop button
//          signal_unblock_by_mask("kb.key.start_oil_pump"); 
            add_task_mb("kb.kbl.start_oil_pump",0,1,0); //led on
          result =0;
          }
      
return n; //return numbers in TASK stack
}

int prog_sw (){
///
///mode2 - low bit
///mode1 - hight bit
///bit map  00, 01, 10, 11
///              1   2   3
///
///control2 - low bit
///control1 - high bit
///bit map  00, 01, 10, 11
///              1  2   3

int mode1=0;
int mode2=0;
int control1=0;
int control2=0;
int post=0;
int mode=0;
int control=0;
int result=0;

       mode1=signal_read("kb.kei1.mode1");   
       mode2=signal_read("kb.kei1.mode2");   
       control1=signal_read("kb.kei1.control1");   
       control2=signal_read("kb.kei1.control2");          
       post=signal_read("kb.kei1.post_conveyor");          
       
       if (mode2 == 1 && mode1 == 0) mode = 1; // mode work
       if (mode2 == 0 && mode1 == 1) mode = 2; // mode upload
       if (mode2 == 1 && mode1 == 1) mode = 3; // mode diagnostic
       
       if (control2 == 1 && control1 == 0) control = 1; //provod
       if (control2 == 0 && control1 == 1) control = 2; //mestno
       if (control2 == 1 && control1 == 1) control = 3; //radio
       
//       if ( mode2 == 0 ) {// diagnostic mode is off
           if (mode == 2 && control == 2) result=1; //oil upload in tank - "Zaka4ka" "Mestno"
           if (mode == 1 && control == 2) result=2; //work at local keyboard - "Rabota" "mestno"
           if (mode == 1 && control == 1) result=3; //work at wire  mode "Rabota" "Provod"
           if (mode == 1 && control == 3) result=4; //work at wire  mode "Rabota" "RPDU"           
//          }
          
           if ( mode == 3 && control == 2 ) result = 5; // diagnostic mode "mestno" is ON
return result;
}



////////////////////////////////// NETWORK ////////////////////////////////////////////

// получение структуры sockaddr, IPv4 или IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
        }
        
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
                
int network_run(char *ip ) {
 int sockfd, numbytes;  
 char buf[MAXDATASIZE];
 struct addrinfo hints, *servinfo, *p;
 int rv;
 char s[INET6_ADDRSTRLEN];

 /*
 if (m_argc != 2) {
     fprintf(stderr,"usage: client hostname \n");
     exit(1);
     }
 */
                                                              
                                                              
     memset(&hints, 0, sizeof hints);
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;
                                                                          
     if ((rv = getaddrinfo(ip, PORT, &hints, &servinfo)) != 0) {
         fprintf(stderr, "getaddrinfo: %s \n", gai_strerror(rv));
         return 1;
          }
                                                                                                  
         // Проходим через все результаты и соединяемся к первому возможному
          for(p = servinfo; p != NULL; p = p->ai_next) {
          
               if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
                   perror("client: socket");
                   continue;
              }
                                                                                                                                                                  
              if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                  close(sockfd);
                  perror("client: connect");
                  continue;
                 }
                                                                                                                                                                                                                      
               break;
            }
                                                                                                                                                                                                                                  
      if (p == NULL) {
          fprintf(stderr, "client: failed to connect \n");
          return 2;
         }
                                                                                                                                                                                                                                                          
         inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
         printf("client: connecting to %s \n", s);
                                                                                                                                                                                                                                                                              
         freeaddrinfo(servinfo); // эта структура больше не нужна
                                                                                                                                                                                                                                                                                  
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
          }
                                                                                                                                                                                                                                                                                                          
        buf[numbytes] = ' ';
                                                                                                                                                                                                                                                                                                              
        printf("client: received '%s' \n",buf);
        //set_date(buf);
                                                                                                                                                                                                                                                                                                                    
        close(sockfd);
return 0;
}
                                                                                                                                                                                                                                                                                                                        

//////////////////////////////////////////////////////////////////////////////////////
int main ()
{

char Sig_Event[100];
int Max_Threads=0;

// time_start();
 init_signal_list();
 init_event_list();
 file_load();
 
 printf("Signals:\n");
 signal_list(); //fill signals and echo to screen
 
 printf("Events:\n");
 event_list(); // check for event empty
// printf("Time: %ld ms\n", time_stop());
// time_start();
// printf ( "Found signals: %i\n\n",signal_update("kb.kei3.telescope_down",100));
// list();
// printf("Time: %ld ms\n", time_stop());
 //printf ("Test oil_pump_stop: %i\n",test_read (31,3,2,0));
  
  int cycle = 0;  
  int state[2];
  while (1)
{
   time_start();
//   mb_read_dCache(int dIndex,char Name[100], int ID, int reg_count)
  // mb_read_dCache(0,"CentralKb",31,6);
   mb_read_dCache(1,"rsrs1rm",32,5);
  // mb_read_dCache(2,"rpdu",33,5);
   mb_read_dCache(3,"rl",34,1);
   mb_read_dCache(4,"rsrs2siren",37,6);
  // mb_read_dCache(5,"adc1",35,8);
  // mb_read_dCache(6,"adc2",36,8);
  // mb_read_dCache(7,"rpduc",73,5);   // rpdu cabel
//   device_list();
//   break;
  
   read_central_kb(); //read mordbus  Keboard

   read_adc1(); // read ADC1
   read_adc2(); // read ADC2
//   signal_state_list(); // print signals > 0
//   signal_state_by_mask("kei");
   //==================
    pthread_t threads[MAX_Signals];
    int status;
    int status_addr;
    int i;
    int total_tasks=-1;
//    someArgs_t args[NUM_THREADS];
//     Discrete_Signals_t args[MAX_Signals]; // Create array of Tasks to control signals
    
//    Discrete_Signals_t input_DS;

     // queue order & stack ?
     // MOZZGGG -|

//     signal_by_mask("rpdu485.");

          printf("total_tasks %i\n ",total_tasks);
  if ( prog_gribok() < 2 ) //check if gribok is NO active
     {
          int prog_num=0;
          

          //prog_test();
          if ( cycle == 2 ) cycle=0;          
          prog_num=prog_sw(); // test selectors mode
	  state[cycle] = prog_num;
          cycle++;
           if ( state[0] != state[1] ) { // switch selector is changed
               printf("NEW MODE SELECTED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
               init_event_list(); //erase modbus event list because selected new mode
               init_task_list();  //erase all previus task
              }

          total_tasks=prog_sound(total_tasks);          

          if ( prog_num == 1 ) { // Zaka4ka MESTNO
          //read_central_kb(); //read mordbus  Keboard
          printf(">>> Zaka4ka Mestno\n");
          total_tasks=prog_oil_pump(total_tasks);
          printf("total_tasks %i\n ",total_tasks);
          }
          
          if ( prog_num == 2 ) { // rabota Mestno
          //init_event_list(); //erase event list          
          //read_central_kb(); //read mordbus  Keboard
          printf(">>> Rabota Mestno\n");
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_support (total_tasks);
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_left_truck (total_tasks);          
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_right_truck (total_tasks);          
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_conv_joy (total_tasks);                    
          printf("total_tasks %i\n ",total_tasks);
          
          total_tasks =  prog_exec_dev_joy (total_tasks);                    
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_telescope_joy (total_tasks);                    
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_sourcer_joy (total_tasks);                    
          printf("total_tasks %i\n ",total_tasks);
          total_tasks =  prog_acceleration (total_tasks);         
          printf("total_tasks %i\n ",total_tasks);
          
          //total_tasks =  prog_hydratation (total_tasks);
          
          total_tasks =  prog_oil_station (total_tasks);
          
          //total_tasks =  prog_reloader (total_tasks);          
          //total_tasks =  prog_conveyor (total_tasks);
          //total_tasks =  prog_stars (total_tasks);          
          //total_tasks =  prog_exec_dev (total_tasks);                    
          }
          
          if ( prog_num == 3 ) 
          {
           printf(">>> Rabota Provod\n");           
           read_wired_rpdu_kb();
          }
          
          if ( prog_num == 4 ) 
          {
           printf(">>> Rabota Radio\n");           
           read_rpdu_kb(); //read radio keyboard
           total_tasks =  prog_rpdu_start_all  (total_tasks);
           total_tasks =  prog_rpdu_oil_station (total_tasks);
           total_tasks =  prog_rpdu_exec_dev_joy (total_tasks);
           total_tasks =  prog_rpdu_conv_joy (total_tasks);
           total_tasks = prog_rpdu_support (total_tasks);
           total_tasks = prog_rpdu_telescope_joy (total_tasks);
           total_tasks = prog_rpdu_sourcer_joy (total_tasks);
           total_tasks = prog_rpdu_acceleration (total_tasks);
           total_tasks = prog_rpdu_sound (total_tasks);           
           //signal_state_by_mask("rpdu485.kei");
          }
          
          if ( prog_num == 5 ) printf(">>> Diagnostic mode\n");          
      }
          //Cycle for input TASKS in THREAD
/*          int gribok=0;
          gribok = signal_read("ad2.adc4_raw_value");
        if ( gribok  < 2100 ) //gribok is Actived
          {
           //old value 24xx
           //new value 2163
           block_all_signals();
           printf("Block All by GRIBOK - [ %i ] < 24xx!\n", gribok);            
            event_update_start_stop("rl.relay2",1,0);
            mb_event_list(); // write to modbus
            //init_signal_list(); //erase all signals
            init_event_list();  //erase all events            
          } 
*/
        /*
        if ( signal_read("ad2.adc4_raw_value") > 2400 && prog_num > 2 ) //unblock ALL when mode "Rabota, and != Mestno"
           {
            unblock_all_signals();
            printf("UNBLOCK ALL SiGNALS by GRIBOK!\n");
            //event_update_start_stop("rl.relay2",0,0); // if start=0 and stop=0 signal not executed by modbus
          }
          */
            Max_Threads =num_task_list();
            NumThreads =   Max_Threads;
            /*
         for (i = 0; i <  Max_Threads; i++) { // Simple TASK
                strcpy(args[i].Input_Signal, "kb.key.start_oil_station"); // Searched name
                // printf("Input Thread Signal Name: %s\n",args[i].Input_Signal);
                strcpy(args[i].Reaction_Signal, "rl.relay1"); // Reaction name
                // printf("Input Thread Signal Search%i: %s\n",i,args[i].Reaction_Signal);
                args[i].SA_ptr = Signal_Array;  //init ptr to signals array
                args[i].MbEv_ptr = Event_Array; //init ptr to events array
             }
     

    */         
     printf ("Tasks create.. \n");          
          for (i = 0; i < Max_Threads; i++) {
              printf("Create Thread %i \n",i);
              status = pthread_create(&threads[i], NULL, DS_thread, (void*) &args[i]);
                  if (status != 0) {
                      printf("main error: can't create thread, status = %d\n", status);
                      exit(ERROR_CREATE_THREAD);
                    }
              }

/*
              input_DS.Input_Signal="kb.key.stop_oil_pump";
              input_DS.SA_ptr = Signal_Array;
              status = pthread_create(&threads[0], NULL,DS_thread , (void*) &input_DS);
                  if (status != 0) {
                      printf("main error: can't create thread, status = %d\n", status);
                      exit(ERROR_CREATE_THREAD);
                      }

*/         

    printf ("Tasks started \n");          
 
     for (i = 0; i < Max_Threads; i++) {
         status = pthread_join(threads[i], (void**)&status_addr);
          if (status != SUCCESS) {
              printf("main error: can't join thread, status = %d\n", status);
              exit(ERROR_JOIN_THREAD);
             }
//          printf("joined with address %i\n", status_addr);
        }
        
   printf("Thread complete \n\n");        
   event_list(); //list all tasks in event array                     
   num_event_list(); //returned number in input task list
                                              
    //  --------------- Network --------------------
    //      network_run("192.168.1.200");
    // --------------------------------------------
                                               
   mb_event_list(); // write to modbus
    //init_event_list(); //clear write buffer <<- very bad if timer signal is started
    num_event_list();
   printf("Time: %ld ms\n", time_stop());
}


return 0;
}
