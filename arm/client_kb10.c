/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>

#define  DEBUG 0 //may be 0,1,2,3
#include "network.h"
#include "signals.h"
#include "speedtest.h"
#include <errno.h>
#include <unistd.h>
#include "hash.h"

#define Devices 40
#define SignalsPerDev 100

#define RD 1
#define WR 2

struct DeviceSignalCache {
char DeviceName[100];
int  Device_ID;
char  Signals_Name[SignalsPerDev][100]; //device_name[100].signal_name[60]
int  Signals_ID[SignalsPerDev]; 
int  Signals_Val[SignalsPerDev];
int  Signals_Ex[SignalsPerDev];
};

struct DeviceSignalCache DevCache[Devices]; //40 devices

int FillDevCache(){
int i=0;
   for (i=0; i < MAX_Signals; i++)
       {
        if (strlen (Signal_Array[i].Name) > 2)
           {
            int dev;
            for (dev=0; dev < Devices; dev++)
                {
                
                 //if device already created
                           if ( DevCache[dev].Device_ID == Signal_Array[i].MB_Id ) //if empty slot for dev
                              {                               
                               int sign=0;
                               int Last_free_index=0;
                               for (sign=0; sign < SignalsPerDev; sign++){
                               
                                    if ( strlen(DevCache[dev].Signals_Name[sign]) < 2 ) { //if signal slot is empty
                                         Last_free_index=sign; //save the index of empty slot
                                        }
                               
                                    if ( strlen(DevCache[dev].Signals_Name[sign]) > 2 ){ //if name not empty
                                         //if ( strstr(DevCache[dev].Signals_Name[sign],Signal_Array[i].Name)!=NULL )// if signal is present
                                             //break;
                                        }                                    

                                        
                                     if ( strstr (DevCache[dev].Signals_Name[sign],Signal_Array[i].Name) == NULL ){    
                                         printf("Free index [%i]\n\r",Last_free_index);
                                         DevCache[dev].Signals_ID[Last_free_index] = i; //signal number in Signal_Array
                                         strcpy(DevCache[dev].Signals_Name[Last_free_index],Signal_Array[i].Name);   
                                         printf("EXIST->>devNum[%i] Mb_ID[%i] DEV_Name[%s] Signal_id[%i] Signal_Name[%s]\n\r ",dev,DevCache[dev].Device_ID ,DevCache[dev].DeviceName, DevCache[dev].Signals_ID[0],DevCache[dev].Signals_Name[0]);                                         
                                         break;
                                         }                  
                                   }
                               }else{  //if not exist
                                      if ( strlen(DevCache[dev].DeviceName) < 2 ) //if empty slot for dev
                                          {
                                             if ( DevCache[dev].Device_ID ==0 ){
                                                 strcpy(DevCache[dev].DeviceName,Signal_Array[i].Name);
                                                 DevCache[dev].Device_ID = Signal_Array[i].MB_Id;
                                                 DevCache[dev].Signals_ID[0] = i; //signal number in Signal_Array
                                                 strcpy(DevCache[dev].Signals_Name[0],Signal_Array[i].Name);
                                                 printf("NEW->>devNum[%i] Mb_ID[%i] DEV_Name[%s] Signal_id[%i] Signal_Name[%s]\n\r ",dev,DevCache[dev].Device_ID ,DevCache[dev].DeviceName, DevCache[dev].Signals_ID[0],DevCache[dev].Signals_Name[0]);
                                                 break;
                                                }
                                           }
                           
                                    }
               }
        
          }
        }
return 0;
}

int ShowDevCache(){
int i=0;
    printf("Show Virtual Device Cache: \n\r");
    for (i=0; i < Devices; i++){
       if (DevCache[i].Device_ID > 0){
          printf(">>>[%i]DevName[%s] DevId[%i] \n\r",i,DevCache[i].DeviceName,DevCache[i].Device_ID);
          int sign;
          for (sign=0; sign < SignalsPerDev; sign++){
               printf("[%i]Signal_id[%i] SignalName[%s]\n\r",sign,DevCache[i].Signals_ID[sign],DevCache[i].Signals_Name[sign]);     
          }
       }
    }
return 0;
}

int sTrigger_Ex (int Signal_Array_id, char *SearchedName, int wait_ExState ){
    if ( strstr(Signal_Array[Signal_Array_id].Name,SearchedName) != NULL)
       {
        if (Signal_Array[Signal_Array_id].ExState == wait_ExState) return 1;        
       }
return 0;
}

int sTrigger_Val (int Signal_Array_id, char *SearchedName, int wait_Val ){
    if ( strstr(Signal_Array[Signal_Array_id].Name,SearchedName) != NULL)
       {
        if (Signal_Array[Signal_Array_id].Value[1] == wait_Val) return 1;        
       }
return 0;
}

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

int Set_Signal_Ex (int ID, char *Signal, char *SearchedName, int Ex){

    if ( strstr(Signal,SearchedName) != NULL)
       {
        Signal_Array[ID].ExState = Ex;        
        return 1;
       }
return  0;
}

int Set_Signal_Ex_Val (int ID, char *Signal, char *SearchedName, int Ex, int Val){

    if ( strstr(Signal,SearchedName) != NULL)
       {
        Signal_Array[ID].ExState = Ex;        
        Signal_Array[ID].Value[1] = Val; 
        return 1;
       }
return  0;
}

int SetVal (char *Signal, int Val, int Ex){
int x=0;
for (x=0; x < MAX_Signals; x++){
    
    if (strstr(Signal_Array[x].Name,Signal) != NULL)
	{
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SETVAL Name{%s} Val{%i} Ex{%i} \n\r",Signal_Array[x].Name,Signal_Array[x].Value[1],Signal_Array[x].ExState);
	Signal_Array[x].Value[1]=Val;
        Signal_Array[x].ExState = Ex;   	
	//return 1;
	}
    }

return 0;
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

int mode1, mode2, control1,control2,alarm_stop1,alarm_stop2,alarm_stop3;
int get_state(){
int state=0;
      int x = 0;
      for (x = 0; x < MAX_Signals; x++)
        {
         if (strstr (Signal_Array[x].Name,"485.kb.kei1.mode1")!=NULL) mode1 = Signal_Array[x].Value[1];
         if (strstr (Signal_Array[x].Name,"485.kb.kei1.mode2")!=NULL) mode2 = Signal_Array[x].Value[1];
         if (strstr (Signal_Array[x].Name,"485.kb.kei1.control1")!=NULL) control1 = Signal_Array[x].Value[1];
         if (strstr (Signal_Array[x].Name,"485.kb.kei1.control2")!=NULL) control2 = Signal_Array[x].Value[1];
         if (strstr (Signal_Array[x].Name,"485.kb.kei1.stop_alarm")!=NULL) alarm_stop1 = Signal_Array[x].Value[1]; //gribok stop
         if (strstr (Signal_Array[x].Name,"485.rpdu485.kei.crit_stop")!=NULL) alarm_stop2 = Signal_Array[x].Value[1]; //gribok stop
         if (strstr (Signal_Array[x].Name,"485.pukonv485c.kei.stop_alarm")!=NULL) alarm_stop3 = Signal_Array[x].Value[1]; //gribok stop
        }
        
        state=mode1+ mode2+ control1+control2+alarm_stop1+alarm_stop2+alarm_stop3; //state as sum of signals value
        //printf (">>>>>>>>>>>>>>>>>>>>>> MODE STATE %i | mode1 %i, mode2 %i, control1 %i,control2 %i,alarm_stop1 %i,alarm_stop2 %i,alarm_stop3 %i   \n\r",state, mode1, mode2, control1,control2,alarm_stop1,alarm_stop2,alarm_stop3);
        if ((mode1 + mode2 + control1 + control2)> 0 ) {
            state = 3;  //work
            printf(" *MODE 3 | ");
           }
           
        if ( ( alarm_stop1 + alarm_stop2 + alarm_stop3) > 0) {
             state = 4; //ALARM stop
             printf(" *GRIBOK STOP!!!| ");
             }
             
if (DEBUG == 1)  printf (">>>>>>>>>>>>>>>>>>>>>> MODE STATE %i | mode1 %i, mode2 %i, control1 %i,control2 %i,alarm_stop1 %i,alarm_stop2 %i,alarm_stop3 %i   \n\r",state, mode1, mode2, control1,control2,alarm_stop1,alarm_stop2,alarm_stop3);
return state;
}


int main(int argc , char *argv[])
{

//*************INIT SIGNALS *********************    
    printf("MAX_Signals [%i] \n",MAX_Signals);
    init_signals_list(); // erase signal lsit 

//************* CREATE SOCKET *********************
 if (argc == 1) {
      printf("No server ip! ip:{%s} \n\r USAGE example: client.exe 192.168.1.1\n\r",argv[1]);
      return;
    }
                
 printf("> Server ip:{%s} \n\r",argv[1]);
                 
                 

 if (socket_init(argv[1]) !=0){
     printf ("No Connection to server\n\r");
     return; //return 0 if all OK else return 1
     }
     
     
//     if (socket_init2() !=0){
//     printf ("No Connection to server\n\r");
//     return; //return 0 if all OK else return 1
//     }
     
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
char tst[MAX_MESS]={0};
char packed_txt_string[MAX_MESS]={0};
int STATE=0;


while (1){
	    printf("THIS IS LoGiC \n\r");

	    speedtest_start(); //time start
	    
	    //======================== read all 485 signals from server create signals and virtual devices ===================

	    memset(message, 0, sizeof(message));
	    //printf("Buffer befor create Messasge: [%s] \n\r",message);
	    frame_pack("rd", "panel10.", message);
	    tcpresult = frame_tcpreq(message); 
			if ( DEBUG == 1 )    printf ("tcp send result[%i]\n\r",tcpresult);	    
				if ( DEBUG == 1 )    printf("=================== ==>   SPEEDTEST Time to prepare packet: [ %ld ] ms. \n\r", speedtest_stop());         		
				if ( DEBUG == 1 )      speedtest_start(); //time start

		
		//in this place need to unpack signals from frame
		printf("RECIVED PACKET [%s]\n\r",signal_parser_buf);
		if ( frame_unpack(signal_parser_buf,tst) < 0){
		     //printf ("ERROR UNPACK! \n\r");
		if ( DEBUG == 1 )     printf(">>>>ERROR FRAME UNPACK! RECIVED^{%s} \n\r",tst);
		    break;
		    }
					printf("=================== ==>   SPEEDTEST Time frame_unpack: [ %ld ] ms. \n\r", speedtest_stop());         
						if ( DEBUG == 1 )	speedtest_start(); //time start					

		if ( DEBUG == 1 ) printf(">>>>RECIVED^{%s} \n\r",tst);
		//printf ("\n\r FRAME_UNPACK: \n\r %s\n\r",tst);
		//Data_to_sName (signal_parser_buf);     	            // explode signals by delimiter ";"	  and copy to Signal.Name[]
				if ( DEBUG == 1 ) speedtest_start(); //time start
		Data_to_sName (tst);		
                memset(signal_parser_buf, 0, sizeof(signal_parser_buf));
                memset(tst, 0, sizeof(tst));
                
                		if ( DEBUG == 1 ) printf("=================== ==>   SPEEDTEST Time load Data_to_sName: [ %ld ] ms. \n\r", speedtest_stop());           	        
  	        		if ( DEBUG == 1 ) speedtest_start(); //time start //deserial test  	         
  	        
	        int z=0;		
	        int id=-1;
	        int wr=0;
	        
	        //FillSignalIndex(); //fill the index of loaded  signals

	        	if ( DEBUG == 1 )        printf("\n\r ================================ *UNPACK signals* ====================================\n\r");	        
	        for (z=0; z < MAX_Signals; z++) {
//////	              printf(" \n\r |Signal FIELDS BEFORE parser: Name{%s} Val0[%i]  Val1[%i]| \n\r",Signal_Array[z].Name, Signal_Array[z].Value[0] , Signal_Array[z].Value[1]); //DEBUG
	              int test=0;	            
	              char buffer[350]={0};
	              
	              strcpy (buffer, Signal_Array[z].Name);
	              test = unpack_signal(buffer  ,z); //from buffer to signal with number Z
	              
	        		if ( DEBUG == 1 )     if (DEBUG == 1)  printf ("FROM SRV ->> [#%i]  Name:[%s]   Val:[%i]     Ex[%i] \n\r",z,Signal_Array[z].Name,Signal_Array[z].Value[1],Signal_Array[z].ExState);	                    
	
    		    }
	        		if ( DEBUG == 1 )        printf(" ================================ *** ====================================\n\r");    		    
                 
				if ( DEBUG == 1 )      printf(" ==>   SPEEDTEST Deserial signals signals: [ %ld ] ms. \n\r", speedtest_stop());     

            		if ( DEBUG == 1 )    printf("=================== ==>  START SWITCH ============================= \n\r");         
	    		if ( DEBUG == 1 )    speedtest_start(); //time start
	        //FillHash(); //fill the array of hash
	        
	        get_state();
	    //    if ((mode1== 1) && ( mode2==1)) STATE =31; //work
	    //    if ((mode1== 1) && ( mode2==0)) STATE =32; //diagnostic
	    //    if ((mode1== 0) && ( mode2==1)) STATE =33; //pump_oil
	        
	        if ((control1== 0) && ( control2==1)) STATE =31; //Mestno
	        if ((control1== 1) && ( control2==1)) STATE =32; //Provod	        
	        if ((control1== 1) && ( control2==0)) STATE =33; //Radio	        
	        
	        
	        z=0;
	        int hydr=0;
	        for (z=0; z < MAX_Signals; z++) {
	                               //printf("\n\r \n\r *******>>>>MODE STATE=[%i] \n\r \n\r",STATE);                     	        
	                               if (Signal_Array[z].Value[1] > 0) printf("NAME{%s} Val{%i} \n\r",Signal_Array[z].Name,Signal_Array[z].Value[1]);
	                               //Set_Signal_Ex (z, Signal_Array[z].Name,"wago.", RD); // read wago	                               	                               
	                               //Set_Signal_Ex (z, Signal_Array[z].Name,"485.kb", RD); //read keyboard modbus device and put result signals in Signal_Array                              
	                               
	                               
	                               
                     switch (STATE){

	                      case 0:  //INIT
	                                		if ( DEBUG == 1 ) printf("\n\r++++++++++++++++++++++++++++++>>>>MODE INIT\n\r");
	                               	 Set_Signal_Ex (z, Signal_Array[z].Name,"wago.", RD); //wago	                               
	                               	 Set_Signal_Ex (z, Signal_Array[z].Name,"485.", RD); //read keyboard
	                               	//Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.sound1_ledms", WR,100);
	                                //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.sound2_ledms", WR,100);
	                                //if ( strstr(Signal_Array[z].Name,"wago.oc_mdo") != NULL ) Set_Signal_Param (z, "wago.oc_mdo", 2 ,1);	                      //stop wago	                               	                               	 
	                      break;         
	                                  
	                      case 1:  //INIT
	                              		if ( DEBUG == 1 ) printf("\n\r++++++++++++++++++++++++++++++>>>>MODE INIT2\n\r");	                      

	                      break;
	               
	               
	                      case 2:  //RESET 
	                              		if ( DEBUG == 1 ) printf("\n\r++++++++++++++++++++++++++++++>>>>MODE RESET\n\r");	                      

	                      break;	               


	                      case 31:  //WORK mestno
	                               //printf("\n\r++++++++++++++++++++++++++++++>>>>MODE WORK MESTNO\n\r");
	                               //STATE = get_state();
	                               //Set_Signal_Ex (z, Signal_Array[z].Name,"wago.", RD); //cmd read wago	                               	                               
	                               //Set_Signal_Ex (z, Signal_Array[z].Name,"485.kb", RD); //cmd read keyboard modbus device and put result signals in Signal_Array                              
	                               //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.state_sound1_led", WR,1); //write bit 
	                               //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.state_sound2_led", WR,1); //write bit 
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs.rm_u1_on", WR,1); //write bit
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs.rm_u2_on", WR,1); //write bit 	                               	                               
	                               
	                               if ( (strstr(Signal_Array[z].Name,"485.kb.key.start_hydratation")!=NULL) && (Signal_Array[z].Value[1] == 1) )// start Hydratation
	                                   { 
	                                     hydr=1;
	                                		if ( DEBUG == 1 )     printf("\n\r====================================================>>>>START Hydrotation\n\r");	                                   
	                                     //SetVal("wago.oc_mdo1.ka",1,WR); //hydro
	                                     //SetVal("485.kb.key.start_hydratation",1,WR); // set state to zerro	                                     	                                    	                                     
	                                     SetVal("485.rsrs2.state_sound2_led",1,WR); //Sound Warning2	                                     	                                     	                               
	                                     SetVal("485.rsrs2.state_sound1_led",1,WR); //Sound Warning1	                   	                                                                                       
	                                    
	                                   }
	                                   
                                       if ((strstr(Signal_Array[z].Name,"485.kb.key.stop_hydratation")!=NULL)  && (Signal_Array[z].Value[1] == 1) ) // stop Hydratation                                       
	                                   { 
	                                     hydr=2;
//	                                     SetVal("485.kb.key.stop_hydratation",0); // set state to zerro	                                   
	                                    		if ( DEBUG == 1 ) printf("\n\r====================================================>>>>STOP Hydrotation\n\r");	   
	                                     //SetVal("485.kb.key.start_hydratation",0,WR); // set state to zerro	                                     	                                    	                                     	                                                                   
	                                     //SetVal("485.rsrs2.state_sound1_led",0,WR); //Sound Warning1	                                     
	                                     //SetVal("485.rsrs2.state_sound2_led",0,WR); //Sound Warning2	                                     	                                   
	                                     //SetVal("wago.oc_mdo1.ka7_1",0,WR); //KOntaktor QF1
	                                     //SetVal("wago.oc_mdo1.ka4_1",0,WR); //Hydro pump M5	                                     
	                                     //SetVal("wago.oc_mdo1.woter1",0,WR); //hydro	                                     
	                                   }
	                                   
	                                   
	                                   if (hydr==1)
	                                   {
	                                    		if ( DEBUG == 1 )  printf("\n\r=============================>>>>START hydr\n\r");	                                   
	                                      //SetVal("wago.oc_mdo1.ka",1,WR); //hydro	                                      
	                                          SetVal("wago.oc_mdo1.ka7_1",1,WR); //KOntaktor QF1
	                                          SetVal("wago.oc_mdo1.ka4_1",1,WR); //Hydro pump M5	                                     
	                                          SetVal("wago.oc_mdo1.woter1",1,WR); //hydro
	                                          
	                                      if ((strstr(Signal_Array[z].Name,"wago.oc_mdo1.ka7_1")!=NULL)  && (Signal_Array[z].ExState > 2) ) // if ExState = 
	                                         {
	                                        		if ( DEBUG == 1 )   printf("STOP SEND HYDRATATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n\r");
	                                           hydr=0;
	                                          }
	                                     }
	                                 
	                                   if (hydr==2)
	                                   {
	                                     		if ( DEBUG == 1 ) printf("\n\r=============================>>>>STOP hydr\n\r");	                                   
	                                      //SetVal("wago.oc_mdo1.ka",1,WR); //hydro
	                                      SetVal("wago.oc_mdo1.ka7_1",0,WR); //KOntaktor QF1
	                                      SetVal("wago.oc_mdo1.ka4_1",0,WR); //Hydro pump M5	                                     
	                                      SetVal("wago.oc_mdo1.woter1",0,WR); //hydro
	                                       if ((strstr(Signal_Array[z].Name,"wago.oc_mdo1.ka7_1")!=NULL)  && (Signal_Array[z].ExState > 2) ) // if ExState = 
	                                          {
	                                    		if ( DEBUG == 1 )       printf("STOP SEND22 HYDRATATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n\r");
	                                           hydr=0;
	                                          }	                                       
	                                     }

	                            
					// ATTENTION!!! SET ExState after analyze execution state!!!!!!!
	                                //Set_Signal_Ex (z, Signal_Array[z].Name,"wago.", RD); //cmd read wago	                               	                               
	                                Set_Signal_Ex (z, Signal_Array[z].Name,"485.kb", RD); //cmd read keyboard modbus device and put result signals in Signal_Array            
	                               
	                      break;

	                      case 32:  //WORK provod
	                            		if ( DEBUG == 1 )    printf("\n\r++++++++++++++++++++++++++++++>>>>MODE WORK PROVOD\n\r");
	                               Set_Signal_Ex (z, Signal_Array[z].Name,"485.kb", RD); //read keyboard modbus device and put result signals in Signal_Array                              	                               
	                               //STATE = get_state();	                               
	                               Set_Signal_Ex (z, Signal_Array[z].Name,"485.rpdu485c.", RD); //read keyboard modbus device and put result signals in Signal_Array
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.state_sound1_led", WR,1); //write bit 
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.state_sound2_led", WR,1); //write bit 
	                               //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs.rm_u1_on", WR,1); //write bit
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs.rm_u2_on", WR,1); //write bit 	                               	                               
	                               //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"wago.oc_mdo", WR,1); //start WAGO
	                               
	                      break;

	                      case 33:  //WORK radio
	                               		if ( DEBUG == 1 )   printf("\n\r++++++++++++++++++++++++++++++>>>>MODE WORK RADIO\n\r");
	                               //STATE = get_state();
	                               Set_Signal_Ex (z, Signal_Array[z].Name,"485.kb", RD); //read keyboard modbus device and put result signals in Signal_Array                              	                               
	                               Set_Signal_Ex (z, Signal_Array[z].Name,"485.rpdu485.", RD); //read keyboard modbus device and put result signals in Signal_Array
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.state_sound1_led", WR,1); //write bit 
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs2.state_sound2_led", WR,1); //write bit 
	                               Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs.rm_u1_on", WR,1); //write bit
	                               //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"485.rsrs.rm_u2_on", WR,1); //write bit 	                               	                               
	                               //Set_Signal_Ex_Val (z, Signal_Array[z].Name,"wago.oc_mdo", WR,1); //start WAGO
	                               
	                      break;

	                      case 4:  //STOP 
	                              		if ( DEBUG == 1 )  printf("\n\r++++++++++++++++++++++++++++++>>>>MODE SOTP !!!!!!!1\n\r");	                      
	                      break;

                              case 5:  //DIAGNOSTIC
	                             		if ( DEBUG == 1 )  printf("\n\r++++++++++++++++++++++++++++++>>>>MODE DIAGNOSTIC !!!!!!!1\n\r");	                                                    
	                      break;

                              case 6:  //ERROR 
	                             		if ( DEBUG == 1 )  printf("\n\r++++++++++++++++++++++++++++++>>>>MODE ERROR !!!!!!!1\n\r");	                                                    
	                      break;
	               
	                      default:  //DEFAULT
	                            		if ( DEBUG == 1 )    printf("default state \n\r");  
	                                //STATE = get_state();
	             }
	             
	                if (DEBUG == 1)  printf (" <<-- TO SRV  [#%i]  Name:[%s]   Val:[%i]     Ex[%i] \n\r",z,Signal_Array[z].Name,Signal_Array[z].Value[1],Signal_Array[z].ExState);	                    	                
	        }
	        
	        
	        /*
	        printf("=================== ==>  Virt Cache Fill.============= \n\r");             
	        FillDevCache();
	        printf("=================== ==>  Show CacheDev: ================ \n\r");         
	        ShowDevCache();
	        break;
	        */
	    		if ( DEBUG == 1 )    printf("=================== ==>  Calculate SWITCH TIME: [ %ld ] ms. \n\r", speedtest_stop());         
         //break; //debug
                     //=========  SEND all signals to TCPCache =======

    		if ( DEBUG == 1 )  speedtest_start ();       //time start
      int x = 0;
//     socket_init();


      memset(message, 0, sizeof(message));     //erase buffer
      //printf("1Must be empty buffer - MESSAGE:[%s] \n\r",message);
      char tmpz[150]={0};
      for (x = 0; x < MAX_Signals; x++)
        {
          if (strlen (Signal_Array[x].Name) > 1)  //write if Name not empty
            { 
             // memset(packed_txt_string, 0, sizeof(packed_txt_string));
             // printf("Founded Name:[%s] \n\r",Signal_Array[x].Name);
              pack_signal (x, tmpz);
              strcat (message, tmpz);
            }
          //else
            //break;              // signals list is end
        }
    		if ( DEBUG == 1 )   printf("=================== ==>  Buffer assembly TIME: [ %ld ] ms. \n\r", speedtest_stop());         
    		if ( DEBUG == 1 )   speedtest_start ();       //time start
      memset(tst, 0, sizeof(tst));
      frame_pack ("wr", message, tst);
    		if ( DEBUG == 1 )  printf("=================== ==>  Calculate PACK TIME: [ %ld ] ms. \n\r", speedtest_stop());         
    		if ( DEBUG == 1 )  speedtest_start ();       //time start
      tcpresult = frame_tcpreq (tst);
      if (DEBUG == 1) printf ("\n\r SEND TST^[%s] \n\r", tst);
    		if ( DEBUG == 1 )  printf ("Status of TCP SEND: [%i]\n\r", tcpresult);
      //tcpsignal_packet_write(message);
      // printf("Send to TCPCache:[%s] \n\r",message);

    	 printf("=================== ==>  Calculate TCP SEND to SRV TIME: [ %ld ] ms. \n\r", speedtest_stop());         
         
    		if ( DEBUG == 1 )    printf("end iteration.. \n\r");     
       }

//*************CLOSE SOCKET***************
 socket_close();


return 0;
}
