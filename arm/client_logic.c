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
#include "signalhash.h"
#include "ringbuffer.h"

#define Devices 40
#define SignalsPerDev 100

#define CONTROL_MASK		0x3
#define CONTROL_RADIO		0x1
#define	CONTROL_MANU		0x2
#define CONTROL_CABLE		0x3

#define MODE_MASK				(0x03 << 2)
#define MODE_DIAG				(0x01 << 2)
#define MODE_PUMP				(0x02 << 2)
#define MODE_NORM				(0x03 << 2)
#define MODE_STOP				128

/*
	 struct WriteBuffer (
	 unsigned int ID;
	 char Name[150];
	 unsigned int ExState;
	 int Value;
	 }
	 struct WriteBuffer W_B[MAX_Signals];


	 int copyW_B (char *sName, char *dName){
	 int z=0;
	 for (z=0; z< MAX_Signals; z++){
	 if (strstr(Signal_Array[z].Name, dName) != NULL)
	 {
	 W_B[z].ID=z; //fill ID
	 memcpy(W_B.Name,Signal_Array[z].Name,(sizeof(Signal_Array[z].Name)/sizeof(char))); //copy Name
	 W_B[z].ExState =Signal_Array[z].ExState;
	 W_B[z].Value = Signal_Array[z].Value;
	 }
	 }

	 return 0;
	 }
 */
unsigned int Send_ID[MAX_Signals];

int Init_Send_ID() {
	int x=0;
	for (x=0; x < MAX_Signals; x++) {
		Send_ID[x]=0;
	}
	return 0;
}

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

//char SignalHash[MAX_Signals][MAX_Signals]; //array for store index of Signals
int Get_Signal_Idx(char *name) {
	struct Signal *s;
	s = hash_find(Signal_Name_Hash, Signal_Array, name);
	if(s) {
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GETVAL Name{%s} Val{%i} \n\r",s->Name,s->Value[1]);		
		return s->Srv_id_num;
	}
	
	return -1;
}

int Get_Signal_Ex(int ID) {
	return Signal_Array[ID].ExState;
}

int Set_Signal_Ex (int ID, int Ex){        
	if(Ex==RD)
	{ 
		if((Signal_Array[ID].TCP_Type[0]=='r') && (Signal_Array[ID].ExState != Ex))
		{
			Send_ID[ID]=1;
			Signal_Array[ID].ExState = Ex;                      
		}
	}

	if(Ex==WR)
	{
		if(Signal_Array[ID].TCP_Type[0]=='w')
		{
			Send_ID[ID]=1;
			Signal_Array[ID].ExState = Ex;        
		}
	}

	return  0;
}

int Set_Signal_Ex_Val(int idx, int Ex, int Val){
	if(Ex == RD) {
		return -1;
	}
	// && (Signal_Array[ID].ExState!=WR)
	struct Signal *s = &Signal_Array[idx];
	if(s) {
		if(s->TCP_Type[0] != 'w')// || (s->Value[1] == Val && s->ExState == Ex))
		{
			return -1;
		}

		Send_ID[s->Srv_id_num] = 1;
		s->ExState = Ex;
		s->Value[1] = Val;
	}
	return  0;
}

int GetVal(char *Signal ){
	struct Signal *s;
	s = hash_find(Signal_Name_Hash, Signal_Array, Signal);
	if(s) {
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GETVAL Name{%s} Val{%i} \n\r",s->Name,s->Value[1]);		
		return 	s->Value[1];
	}

	return -1;
}


int FillSignalIndex(void){ //fill the id of signals in cyrrent signals list;

	int n=0;
	for (n=0; n < MAX_Signals; n++){
		Signal_Array[n].Srv_id_num = n;  
	}
	return 0;
}

int Get_State(){
	int state=0;
	int x = 0;
	struct Signal *s;
	int mode1 = 0, mode2 = 0, control1 = 0,control2 = 0,alarm_stop1 = 0,alarm_stop2 = 0,alarm_stop3 = 0;


	s = hash_find(Signal_Name_Hash, Signal_Array, "485.kb.kei1.mode1");
	if(s) mode1 = s->Value[1];
	s = hash_find(Signal_Name_Hash, Signal_Array, "485.kb.kei1.mode2");
	if(s) mode2 = s->Value[1];

	s = hash_find(Signal_Name_Hash, Signal_Array, "485.kb.kei1.stop_alarm"); //gribok stop
	if(s) alarm_stop1 = s->Value[1];
	s = hash_find(Signal_Name_Hash, Signal_Array, "485.rpdu485.kei.crit_stop"); //gribok stop
	if(s) alarm_stop2 = s->Value[1];
	s = hash_find(Signal_Name_Hash, Signal_Array, "485.kb.pukonv485c.stop_alarm"); //gribok stop
	if(s) alarm_stop3 = s->Value[1];

	control1 = GetVal("485.kb.kei1.control1");
	control2 = GetVal("485.kb.kei1.control2");

	state = control1 | (control2 << 1) | (mode1 << 2) | (mode2 << 3);

	//printf (">>>>>>>>>>>>>>>>>>>>>> MODE STATE %i | mode1 %i, mode2 %i, control1 %i,control2 %i,alarm_stop1 %i,alarm_stop2 %i,alarm_stop3 %i   \n\r",state, mode1, mode2, control1,control2,alarm_stop1,alarm_stop2,alarm_stop3);
	if ((alarm_stop1 | alarm_stop2 | alarm_stop3)) {
		printf(" *GRIBOK STOP!!!|\n");
		return state | 128;
	}

	//if ((control1== 0) && ( control2==1)) state =31; //Mestno
	//if ((control1== 1) && ( control2==1)) state =32; //Provod	        
	//if ((control1== 1) && ( control2==0)) state =33; //Radio	        

	return state;
}

int GetSignals() {
	int z=0;		
	int tcpresult;
	char tst[MAX_MESS]={0};

	memset(message, 0, sizeof(message));
	//printf("Buffer befor create Messasge: [%s] \n\r",message);
	frame_pack("rd", ".", message);
	tcpresult = frame_tcpreq(message); 

	//in this place need to unpack signals from frame
	if(frame_unpack(signal_parser_buf, tst) < 0){
		return -1;
	}

	Data_to_sName(tst);
	memset(signal_parser_buf, 0, sizeof(signal_parser_buf));
	memset(tst, 0, sizeof(tst));

	for (z=0; z < MAX_Signals; z++) {
		//////	              printf(" \n\r |Signal FIELDS BEFORE parser: Name{%s} Val0[%i]  Val1[%i]| \n\r",Signal_Array[z].Name, Signal_Array[z].Value[0] , Signal_Array[z].Value[1]); //DEBUG
		int test=0;	            
		char buffer[350]={0};
		strcpy (buffer, Signal_Array[z].Name);
		test = unpack_signal(buffer, z); //from buffer to signal with number Z
		Signal_Array[z].Srv_id_num = z;
	}

	INIT_HASH_MAPS();
}

int Init (){
	struct Signal *s;
	s = hash_find(Signal_Name_Hash, Signal_Array, "485.kb.kei1.power");
  Pressure_Show();
	Oil_Show();
	Metan_Show();
	if(Get_Signal_Ex(Get_Signal_Idx("wago.oc_mdi.err_phase")) == RD)
		return 0;
	if(Get_Signal("wago.oc_mdi.err_phase")) {
		printf("Phase error!\n");
		return 0;
	}

	if(Get_Signal_Ex(Get_Signal_Idx("wago.bki_k1.M1")) == RD)
		return 0;
	if(Get_Signal_Ex(Get_Signal_Idx("wago.bki_k2.M2")) == RD)
		return 0;
	if(Get_Signal_Ex(Get_Signal_Idx("wago.bki_k3_k4.M3_M4")) == RD)
		return 0;
	if(Get_Signal_Ex(Get_Signal_Idx("wago.bki_k5.M5")) == RD)
		return 0;
	if(Get_Signal_Ex(Get_Signal_Idx("wago.bki_k7.M7")) == RD)
		return 0;
	if(Get_Signal("wago.bki_k1.M1") ||
		 Get_Signal("wago.bki_k2.M2") ||
		 Get_Signal("wago.bki_k3_k4.M3_M4") ||
		 Get_Signal("wago.bki_k5.M5") ||
		 Get_Signal("wago.bki_k7.M7"))
	{
		printf("BKI error!\n");
		return 0;
	}

	if(Get_Signal("wago.oc_mdi1.oc_w_qf1")) {
		printf("Initialization completed!\n");
		return 1;
	}

	Set_Signal_Ex_Val(Get_Signal_Idx("wago.oc_mdo1.ka7_1"), WR, 1);

	return 0;
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

	ring_buffer_init(&Signal_Mod_Buffer);

	if (socket_init(argv[1]) !=0){
		printf ("No Connection to server\n\r");
		return; //return 0 if all OK else return 1
	}

	printf("Initializing client logic\n");

	//******************* WORK CYCLE *******************
	int delay = 0; //1 cycle 1 ms
	char packed_txt_string[MAX_MESS]={0};
	int STATE=0;
	int RqTCPSend=0;
	char tst[MAX_MESS]={0};
	int tcpresult, oldMode = 0;
	int workerInitialized = 0;
	int initialized = 0;
	Init_Send_ID();

	while (1){
		//printf("THIS IS LoGiC \n\r");

		speedtest_start(); //time start

		//======================== read all 485 signals from server create signals and virtual devices ===================

		//if ( DEBUG == 1 )        printf(" ================================ *** ====================================\n\r");    		    

		GetSignals();

		if(!workerInitialized) {
			Init_Worker();
			workerInitialized = 1;
		}

		if(!initialized) {
			initialized = Init();
			goto UpdateSignals;
		}

		/////////////////////////////////////////////////////// COMMAND/////////////////////////////////////////////////////////////

		//if ( DEBUG == 1 )    printf("=================== ==>  START SWITCH ============================= \n\r");         

		// Check state
		int hydr=0;

		struct hash_item_s *item;
		struct Signal *s;

		STATE = Get_State();

		if(STATE & 128) {
			Process_RED_BUTTON();
			STATE = 0;
		}
		//printf("\n\rAnalyzing state %x", STATE);
		//printf("\n\r          mode  %x (%x)\n\r", STATE & MODE_MASK, MODE_PUMP);
		
		if(oldMode != (STATE & MODE_MASK)) {
			Process_Mode_Change();
			oldMode = STATE & MODE_MASK;
		}

		switch(STATE & MODE_MASK) {
			case MODE_NORM:
				Process_Normal();
				switch(STATE & CONTROL_MASK){
					case CONTROL_MANU:  //INIT
						Process_Local_Kb();
						break;         

					case CONTROL_CABLE:  //INIT
						Process_Cable_Kb();
						break;

					case CONTROL_RADIO:  //RESET 
						Process_Radio_Kb();
						break;	               

					default:  //DEFAULT
						if ( DEBUG == 1 )    printf("default state \n\r");  
				}
				break;
			case MODE_PUMP:
				Process_Pumping();
				break;
			case MODE_DIAG:
				Process_Diag();
				switch(STATE & CONTROL_MASK){
					case CONTROL_MANU:  //INIT
						Process_Local_Kb();
						break;         

					case CONTROL_CABLE:  //INIT
						Process_Cable_Kb();
						break;

					case CONTROL_RADIO:  //RESET 
						Process_Radio_Kb();
						break;	               

					default:  //DEFAULT
						if ( DEBUG == 1 )    printf("default state \n\r");  
				}
				break;
				break;
		}

UpdateSignals:
		// Read keyboard
		item = hash_find_by_prefix(Signal_Prefix_Hash, "485.");
		while(item) {
			if(strncmp(Signal_Array[item->idx].Name, "485.kb.", 7) == 0)
				Set_Signal_Ex(item->idx, RD); //cmd read keyboard modbus device and put result signals in Signal_Array            
			if(strncmp(Signal_Array[item->idx].Name, "485.rsrs.", 9) == 0)
				Set_Signal_Ex(item->idx, RD); //cmd read keyboard modbus device and put result signals in Signal_Array            						
			if(strncmp(Signal_Array[item->idx].Name, "485.rpdu485.", 9) == 0)
				Set_Signal_Ex(item->idx, RD); //cmd read keyboard modbus device and put result signals in Signal_Array            						
			item = item->next;
		}
		item = hash_find_by_prefix(Signal_Prefix_Hash, "wago.");
		while(item) {
			Set_Signal_Ex(item->idx, RD); //read wago	                               
			item = item->next;
		}

		int idx, value, st;
		while(ring_buffer_get(Signal_Mod_Buffer, &idx, &value, &st)) {
			if(idx < 0) continue;
			if(st == RD) {
				Set_Signal_Ex(idx, st);
			} else {
				printf("Writing signal %s: %d (%d)\n", Signal_Array[idx].Name, value, st);
				Set_Signal_Ex_Val(idx, st, value);
			}
			ring_buffer_pop(Signal_Mod_Buffer);
		}

		////////////////////////=========  SEND all signals to TCPCache =======//////////////////////////////////////

		if(DEBUG == 1) speedtest_start(); //time start
		int x = 0;
		//     socket_init();
		message[0] = 0;
		//memset(message, 0, sizeof(message));     //erase buffer
		//printf("1Must be empty buffer - MESSAGE:[%s] \n\r",message);
		char tmpz[150]={0};
		int Send_Ready=0;
		for(x = 0; x < MAX_Signals; x++)
		{
			if(Signal_Array[x].Name[0] == 0)  //write if Name not empty
			{ 
				break;
			}

			if(Send_ID[x]){
				if(x > 356) printf("Writing signal %s [%d]\n", Signal_Array[x].Name, x);
				pack_signal(x, tmpz);
				strcat(message, tmpz);                 
				Send_Ready=1;
			}

			Send_ID[x]=0;
		}
		if (Send_Ready == 1){
			memset(tst, 0, sizeof(tst));
			frame_pack ("wr", message, tst);
			tcpresult = frame_tcpreq (tst);
		}

		//printf("=================== ==>  Calculate TCP SEND to SRV TIME: [ %ld ] ms. \n\r", speedtest_stop());         

		//if ( DEBUG == 1 )    printf("end iteration.. \n\r");     
	}

	//*************CLOSE SOCKET***************
	socket_close();


	return 0;
}
