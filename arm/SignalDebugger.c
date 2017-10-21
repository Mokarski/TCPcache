/*
 *   C ECHO client example using sockets
 */
#include <stdio.h>		//printf
#include <string.h>		//strlen
#include <sys/socket.h>		//socket
#include <arpa/inet.h>		//inet_addr
#include <stdlib.h>
//#include "/home/opc/Kombain/test/include/modbus/modbus.h"
//3
#define DEBUG 0 // may be set to 0,1,2,3,4
#include "network.h"
#include "signals.h"
//#include "virtmb.h"
#include "speedtest.h"
#include <errno.h>


int Send_Signal[MAX_Signals] = {0};






//*********************************Read_Op ****************************************
int Read_Op(char *str){

	char tst[MAX_MESS]={0};
	int ret;
	int rd_wr;
	int tcpresult = 0;

	speedtest_start ();	//time start      
	memset(signal_parser_buf, 0, sizeof(signal_parser_buf)); //erase buffer for next iteration
	memset(message, 0, sizeof(message));
	//======================== read all 485 signals from server create signals and virtual devices ===================

	frame_pack ("rd", str, message);
	printf("Message: %s\n\n", message);
	tcpresult = frame_tcpreq (message);
	//ret=tcpresult; //return result of unpack

	//printf ("Status of TCP SEND Read request: [%i][%s]\n\r", tcpresult,message);

	if (tcpresult > 1)
	{ 
		rd_wr = frame_unpack(signal_parser_buf, tst); //rd_wr ==3 then OK
	} else printf("!!!ERROR frame_tcpreq = %i \n\r",tcpresult);
	ret=rd_wr;

	if (rd_wr != 3) {
		printf("!!!ERR unpack code[%i] \n\r",rd_wr);
		return -1 ; //no OK from server
	}

	int signals_found = 0;
	signals_found = Data_to_sName (tst); //extract all signals from buffer and put into Signal.Name field
	if (signals_found < 1) {
		printf("!!!ERR no signals found [%i] \n\r",signals_found);
		return -2 ; //no signals found
	}

	//****************** UNPACK signals **************************
	int z = 0;      
	for (z = 0; z < signals_found; z++) //create virt devices and unpack signals to struct Signal_Array[]
	{	
		char buffer[350] = "";
		int test = 0;
		strcpy (buffer, Signal_Array[z].Name);
		test = unpack_signal (buffer, z);	//UnPACK Signal from buffer to signal with number Z

		//if ( DEBUG == 1 ) printf(">>mb_fill Nmae[%s] Id[%i] Register[%i] Ex[%i]\n\r",Signal_Array[z].Name, Signal_Array[z].MB_Id, Signal_Array[z].MB_Reg_Num, Signal_Array[z].ExState);	
		if (Signal_Array[z].Name[0])
		{
			printf ("[%i]-Name:[%s]\t\tVal:[%i]\tEx:[%i]\tType[%s]\tVersion[%i]\r\n",z,Signal_Array[z].Name, Signal_Array[z].Value[1], Signal_Array[z].ExState,Signal_Array[z].TCP_Type,Signal_Array[z].TCP_Addr);
		}
		Send_Signal[z] = 1;

	} //end for
	//****************** END UNPACK***************************************

	//printf("RqTime:[%ld]ms. \n\r",	 speedtest_stop ());


	return ret;
}
//*********************************************************************************


//*********************************Write_Op ****************************************
int Write_Op(){
	int tcpresult = 0;
	char tst[MAX_MESS]={0};      
	char send_buf[MAX_MESS]={0}; 
	char tmpz[150]={0};     

	speedtest_start ();	//time start     
	int x = 0;
	int ready_to_send_tcp=0;

	//erase buffer     
	memset(message, 0, sizeof(message));
	memset(tst, 0, sizeof(tst));
	for (x = 0; x < MAX_Signals; x++)
	{
		if ( (Signal_Array[x].Value[1] > 0) || (Signal_Array[x].ExState > 0) )
			if ( DEBUG == 3 )  printf ("[%i]TO_SRV  <<-- Name:[%s] Value:[%i] ExState:[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState);

		if  (Signal_Array[x].Value[1] > 0) 
			if ( DEBUG == 1 )  printf ("[%i]TO_SRV  <<-- Name:[%s] Value:[%i] ExState:[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState);

		if  (Signal_Array[x].Value[1] > 0) 
			if ( DEBUG == 4 )  printf ("[%i]TO_SRV  <<-- Name:[%s] Value:[%i] ExState:[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState);

		if ((Signal_Array[x].Name[0])){
			printf ("[%i]-- Name:[%s] Value:[%i] ExState:[%i] Type[%s] Version[%i]\n\r ", x,Signal_Array[x].Name, Signal_Array[x].Value[1], Signal_Array[x].ExState,Signal_Array[x].TCP_Type,Signal_Array[x].TCP_Addr);
		}

		if (strlen (Signal_Array[x].Name) > 2 && Send_Signal[x]) //write if Name not empty
		{		
			pack_signal (x, tmpz);
			strcat (tst, tmpz);
			ready_to_send_tcp=1;
			Send_Signal[x] = 0;
		}
		else
			break;		// signals list is end
	}

	//printf("SEND_BUFFER[%s]\n\r",tst);
	if (strlen(tst) > 0){
		frame_pack ("wr",tst, send_buf);
		if (ready_to_send_tcp == 1) {
			tcpresult = frame_tcpreq (send_buf); //send to srv
		}
	}else printf("ERR!!! Send_Buffer is empty! \n\r");

	if (DEBUG == 3) printf ("\n\r SEND WRITE request TST^[%s] \n\r", send_buf);
	printf ("Status of TCP SEND: [%i]\n\r", tcpresult);
	printf
		(" ++++++++++++++++++++++++==>   SPEEDTEST Send to TCPCache Time: [ %ld ] ms. \n\r",	 speedtest_stop ());

	return tcpresult;
}
//*********************************************************************************

str_filter(char *str){
	int pos=strlen(str)- 1;
	if ( str[pos] == '\n')
		str[pos]='\0';
}

int main (int argc, char *argv[])
{
	int TCP_Ready_SEND;
	int TCP_State;
	char TestSignal[MAX_Signals][50];

	if (argc == 1) {
		printf("No server ip! ip:{%s} \n\r USAGE example: client.exe 192.168.1.1\n\r",argv[1]);
		return;
	}

	printf("> Server ip:{%s} \n\r",argv[1]);




	//INIT SIGNALS     
	if (DEBUG == 1)   printf ("MAX_Signals [%i] \n", MAX_Signals);
	init_signals_list ();		// erase signal lsit 
	//if (socket_init ("127.0.0.1") != 0)
	if (socket_init (argv[1]) != 0)
	{
		printf ("NO Connection to server\n\r");
		return;
	}


	FILE *fp;
	char str[128];
	if ((fp=fopen("signals_test.cfg", "r") )==NULL) {
		printf("Cannot open file-> signals_test.cfg. Fill the file signal names.\n");
		exit (1);
	}
	int cnt=0;
	while(!feof(fp)) {
		if (fgets(str, 126, fp)) {
			printf("%s \n", str);
			str_filter(str);
			strcpy(TestSignal[cnt],str);
			cnt++;
		}
	}
	fclose(fp);

	int i=0;
	printf ("***#SIGNAL DEBUGGER#****\n\r");          
	printf("\x1B[2J");
	printf("\x1B[1;1H");
	while (1)
	{
		if (i < cnt)
		{
			printf ("TEST SIGNAL:>>{%s}<<\n \n",TestSignal[i]);
			TCP_Ready_SEND = Read_Op(TestSignal[i]);
			if (TCP_Ready_SEND < 0) break;
			i++;
		} else {
			//printf("\x1B[1;1H");
			printf("\x1B[1;1H");
			i=0;
		}
	}
	socket_close ();

	return 0;
}
