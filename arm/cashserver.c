/*
	 Cash TCP Server for data exchange between Logic_Core, Panel_Keyboard, Web-client;
 ***********************************************************************************
 Created in May, 2017. Author: "Mokar",tomsknets@yandex.ru; "'SPARC' Company"(c).
 */

#include<stdio.h>
#include<string.h>    
#include<stdlib.h>     
#include<string.h>        
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<unistd.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/select.h>
#include<pthread.h>   //for threading , link with lpthread
#include<time.h>      // for time_t
#include<errno.h>     // for print errors

#define DEBUG 0       // 0 -not debug  1,2,3,4,5- debug
#include "signals.h"
#include "network.h"
#include "speedtest.h"
#include "signalhash.h"

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS               0
#define FAIL_ERROR			  1
#define NUM_THREADS 			  4
#define MAX_SIZE			100
#define RFC1123FMT 	"%a, %d %b %Y %H:%M:%S GMT"
#define PORT 		8888
#define MAX_CONN 10
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int eventsock;

// ********************************** Массив сигналов **********************************

typedef struct Discrete_Signals { // store one  signal state
	char Input_Signal[150];        //searched signal name
	char Reaction_Signal[100];     //reaction signal name
	//    char Buf_Signal[100];          //buffer for parser
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
	int nSock;
	//add mutex 
} Discrete_Signals_t;

//Discrete_Signals_t args; // Create array of Tasks to control signals by threads
Discrete_Signals_t args; // init for pthread can read and write global Signal_Array                         
int client_sockets[MAX_CONN];
struct hash_s *prefix_hash;
struct hash_s *name_hash;


void * getglobalsignals() { return (void*)&args; }
void *connection_handler(void *);



//**************************** MAIN SECTION ************************************************** 

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , *new_sock;
	struct sockaddr_in server , client;
	int sc, event[2];
	//someArgs_t args;

	args.SA_ptr = Signal_Array;
	int status,status_addr;
	char sepDotComma [10] = ";";
	char sepComma [10] = ",";
	char *iStr;
	char *pSB;

	memset(client_sockets, 0, sizeof(client_sockets));

	//Load signals
	init_signals_list();
	signals_file_load();
	hash_create(&prefix_hash);
	hash_create(&name_hash);

	for(sc = 0; sc < MAX_Signals; sc ++) {
		if(Signal_Array[sc].Name[0] == 0) {
			break;
		}
		Signal_Array[sc].Srv_id_num = sc;
		if(Signal_Array[sc].TCP_Type[0] == 'r') {
			Signal_Array[sc].ExState = 1;
		}
		hash_add_by_prefix(prefix_hash, Signal_Array, sc);
		hash_add(name_hash, Signal_Array, sc);
	}

	printf("Signals:\n\n\r");
	print_signals_list(); //fill signals and echo to screen

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return FAIL_ERROR;
	}
	puts("bind done");
	printf("Server Listen SOcket #ID[%i] \n\r",socket_desc);

	//Listen
	listen(socket_desc , 25); // queue of clients = 6

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	socketpair(AF_LOCAL, SOCK_STREAM, 0, event);
	eventsock = event[1];

	pthread_t sniffer_thread;
	pthread_attr_t threadAttr;      // Set up detached thread attributes
	pthread_attr_init(&threadAttr);
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(&sniffer_thread, &threadAttr, connection_handler, (void*) client_sock) < 0)
	{
		perror("SERVER: Could not create thread");
		return (ERROR_CREATE_THREAD);
	}

	char *evbuf = "w";

	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		printf("\n\r ***TCPSERVER: \n\r");
		puts("Connection accepted");
		printf("\n\r>>>>>    (MAIN While) CLient socket ID[#%i] \n\r",client_sock); 

		for(sc = 0; sc < MAX_CONN; sc ++) {
			if(client_sockets[sc] == 0) {
				client_sockets[sc] = client_sock;
				write(event[0], evbuf, 1);
				break;
			}
		}

		/*
			 for (s=0; s < MAX_Signals; s++){
			 printf ("",	Signal_Array[s].Name, Signal_Array[s].Value[1] );
			 }
		 */


		// структура аргументов для записи сокета и результата обработки
		args.nSock = client_sock;
		//args.pSignal = Signals;
		args.SA_ptr = Signal_Array;
		//if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) &args[0]) < 0)
		puts("\nSERVER: Handler assigned"); 

	}


	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}

	close(socket_desc);
	//	close(client_sock);
	close(c);

	return SUCCESS;
}
/*
 * This is substract from src string to destination string n symbols
 * Копирует в dst не больше cnt символов из src начиная с позиции pos.
 * Память под принимающую строку должна быть заранее выделена с учётом завершающего нуля. 
 */
/*
	 char * substr(char * dst, const char * src, size_t pos, size_t cnt){
	 size_t len;

	 if ( ! dst || ! src || strlen(src) < pos )
	 return NULL;

	 if ( ( len = strlen(src + pos) ) > cnt )
	 len = cnt;
	 strncpy(dst, src + pos, len);
	 dst[len] = '\0';

	 return dst;
	 }
 */
/*
 * приведение целого к строковому формату
 */
/*
	 void Reverse(char s[]) {
	 int c, i, j;
	 for ( i = 0, j = strlen(s)-1; i < j; i++, j--) {
	 c = s[i];
	 s[i] = s[j];
	 s[j] = c;
	 }
	 }
	 void ItoA(int n, char s[]) {
	 int i, sign;
	 sign = n;

	 i = 0;
	 do {
	 s[i++] = abs(n % 10) + '0';
	 } while ( n /= 10 );
	 if (sign < 0)
	 s[i++] = '-';
	 s[i] = '\0';
	 Reverse(s);
	 }
 */



int ShowAllSignals(){
	int i=0;
	for (i=0; i < MAX_Signals; i++){
		printf("#%i Signlal[%s] \n\r",i,Signal_Array[i].Name);
	}
	return 0;
}

//********************************** READ **********************************************
int Read_operation(char tst[MAX_MESS], char to_send[MAX_MESS]){
	char *istr;                
	char sep_comma[4]=";";
	int found=0;  
	//printf("[recived >CMD READ] read_signal \n\r");		    
	size_t xx=0;
	size_t cnt=0;
	struct hash_item_s *item;
	//printf ("SERVER: recive from client: [%s]\n\r",tst);
	istr = strtok(tst,sep_comma);
	if (istr != NULL){
		printf ("SERVER: GET the NAME [%s]\n\r",istr);
	}

	char tmpz[150]={0};
	//strcpy (result,""); //erase buffer
	if(*istr == '.') {
		int i;
		for(i = 0; i < MAX_Signals; i ++) {
			if(Signal_Array[i].Name[0] == 0) {
				break;
			}
			pack_signal(i,tmpz);
			strcat (to_send,tmpz);
			xx++;
			found++;
		}
	} else {
		int len = strlen(istr);
		item = hash_find_by_prefix(prefix_hash, istr);
		//printf("Hash items found: %p\n", item);

		while(item) {
			if(strncmp(Signal_Array[item->idx].Name, istr, len) == 0) {
				pack_signal(item->idx,tmpz);
				strcat(to_send,tmpz);
				xx++;
				found++;
			}
			item = item->next;
		}
	}

	//printf("Signals READ [%s] found [%i]! \n\r", istr, found);
	//if (found == 0) ShowAllSignals();
	return found; //founded signal counter
}
//**************************************************************************************


//********************************** WRITE *********************************************
int Write_operation (char tst[MAX_MESS]){
	char *istr;
	int found=0;          //flag how many founded signals
	char digit[5];    //buffer Value of signal as CHAR
	char sname [150]={0}; //buffer for temp store signal NAME
	char buf_signals[MAX_Signals][150]={0}; //array of MAX_signal elements AND 150 characters each
	int t=0;		        
	int val;
	size_t xx=0;
	size_t cnt=0;					    

	//printf("[recived >CMD WRITE] write_signal \n\r");
	int sn=1; //number of signals started from 1, because before while we get 1 signal

	//if (DEBUG == 1) printf ("SERVER: recive from client WRITE Req [%s]\n\r",tst);
	istr = strtok(tst, ";");			            
	if (istr != NULL) {
		strcpy(buf_signals[0],istr);
	} else {printf ("Not found delimiter [;]\n\r");}

	while ( istr != NULL ){
		istr = strtok(NULL, ";");
		if (istr == NULL) // defend from sigfault
		{
			//printf(" End write to cachebuf, at position [%i]\n\r",sn);
			break;
		} else {			            
			strcpy(buf_signals[sn], istr);
			//if (DEBUG == 1) printf ("Explode ; to cachebuf [#%i]  NAME: [%s] \n\r",sn,buf_signals[sn]); //debug
		}

		sn++;//????
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>SN [%i]\n\r",sn);
	}

	//if (DEBUG == 1) printf("Signal_counter sn=%i\n\r",sn);		
	int pr=0;
	int utest;
	printf("Signals written:\r\n");
	for(pr=0; pr < sn; pr++) //cycle for signals
	{ 
		if(buf_signals[pr][0] == 0) {
			break;
		}

		char tmpz[190]={0};
		char *istrName = strchr(buf_signals[pr], ':');

		if(istrName != NULL){
			//printf("istr %s \n\r",istrName);
			*istrName = 0;
			strcpy(tmpz, buf_signals[pr]);
			*istrName = ':';
		} else {
			printf ("Not found Name in buf_signals[:]\n\r");
			continue;
		}

		struct Signal *s = hash_find(name_hash, Signal_Array, tmpz);
		if(s) {
			found++;
			utest = unpack_signal(buf_signals[pr], s->Srv_id_num); //unpack from field buffer to signal properties fields ????
			if(s->ExState == 2) printf("%s:%d:%d\n", s->Name, s->ExState, s->Value[1]);
		}
	}
	//printf("Signals WRITE  found [%i]! \n\r",found);
	return found; //return number of written signals
}
//**************************************************************************************

/*
 * This will handle connection for each client
 * */
void* connection_handler(void *args)
{
	fd_set socks;
	// from struct
	//		
	while(1) {
		Discrete_Signals_t *arg = (Discrete_Signals_t * ) getglobalsignals();
		FD_ZERO(&socks);
		int n=0, i, maxfd = eventsock;

		FD_SET(eventsock, &socks);

		for(i = 0; i < MAX_CONN; i ++) {
			if(client_sockets[i] != 0) {
				maxfd = maxfd > client_sockets[i] ? maxfd : client_sockets[i];
				FD_SET(client_sockets[i], &socks);
			}
		}

		if(select(maxfd + 1, &socks, NULL, NULL, NULL) < 0) {
			continue;
		}

		if(FD_ISSET(eventsock, &socks)) {
			char localbuf[127];
			read(eventsock, localbuf, sizeof(localbuf));
		}

		for(i = 0; i < MAX_CONN; i ++) {
			//Get the socket descriptor
			//int sock = arg->nSock; //problem one id to many threads
			int sock = client_sockets[i]; // client_sock

			if(sock == 0) {
				continue;
			}

			if(!FD_ISSET(sock, &socks)) {
				continue;
			}

			//printf(">>>> THREAD Socket ID[#%i] \n\r",sock);
			//
			int read_size;
			char *mesOk, *mesNo, *mesErr, *mesBad, *mesUnp;
			char  client_message[MAX_MESS]={0};
			//    char  client_message_write[MAX_MESS];    
			char  signalsBuffer[MAX_MESS]={0};
			char *cmd_end = ";";
			mesOk = "Ok!";	
			mesBad = "NOcmd!";
			mesErr = "Err!";
			mesUnp = "Err! unpack Frame!";
			//    char a[4096];
			char dig[128];	
			//char packed_txt_string[40000];
			char tst[MAX_MESS]={0};
			int rd_wr=0;
			//    int found=0;  //founded signals counter
			int iterration=0;
			if( (read_size = recv(sock , client_message , MAX_MESS , 0) ) > 0 )
			{
				//if (DEBUG == 1) iterration++;
				int mess_length = sizeof(client_message) / sizeof(char);
				//if (DEBUG == 1) printf("\n\r {inTHREAD_sock[%i] Cycle %i} \n\r [SRV received: %i bytes] client_message_read: [%s]\r\n",sock,iterration,read_size ,client_message);
				//if (strlen(client_message) < 30) printf(">>>[Sock %i] Recived from client[%s]\n\r",sock,client_message);
				//printf("Received %d bytes from socket %d\n", read_size, i);


				//********************************* COMMAND SELECTION AND EXECUTION ******************************/
				//pthread_mutex_lock(&mutex); // block mutex 
				memset(tst, 0, sizeof(tst));
				rd_wr = frame_unpack(client_message, tst);
				//pthread_mutex_unlock(&mutex); //unlock mutex

				if (rd_wr < 0) {
					//printf ("\n ===============================================================\n");
					//printf ("----------------------> FRAME_UNPACK ERROR!!!!!: %i \n\r",rd_wr);
					//if (DEBUG > 0) printf("\n\rSOCKET_ID[%i] {Cycle %i} \n\r [SRV received: %i bytes] client_message: [%s]\r\n",sock,iterration,read_size ,client_message);
					//printf ("\n ===============================================================\n");
					write(sock, mesUnp, strlen(mesUnp));
					close (sock); // close socket before exit
					printf("Drop the socket[%i] and close connection. \n\r",sock);
					return 0; 
				}

				switch (rd_wr){
					case 1:  //READ OPERATION
						if (DEBUG == 1) speedtest_start();
						//pthread_mutex_lock(&mutex); // block mutex 
						char result[MAX_MESS]={0};  //buffer for response 50 000 bytes
						char result2[MAX_MESS]={0};  //buffer for response 50 000 bytes
						memset(client_message, 0, sizeof(client_message)); //erase buffer before fill the result					
						if (Read_operation(tst,client_message) > 0) { //if founded requested name of signals


							frame_pack("Ok!",client_message,result2); //pack all info to FRAME
							//if (DEBUG == 1) printf ("result2: {%s}",result2);
							int msg_len = strlen(result2);
							int write_ok;
							// pthread_mutex_unlock(&mutex); //unlock mutex

							write_ok = write(sock, result2, msg_len ); //send packet to client			
							//if (DEBUG == 1) printf("\n\rTry to Write,  Sendded bytes: [%i] \n\r",write_ok);

							if (write_ok < 0){
								printf("TCP SEND Error description is : %s\n",strerror(errno));			    
							}				     
							memset (client_message, 0, mess_length);

						} else {  // if signals not found
							//printf("CMD_READ: Signal not found \n\r");
							write(sock, mesErr, strlen(mesErr));			
							memset(client_message, 0, mess_length);
						}
						//if (DEBUG == 1) printf(" ++++++++++++++++++++++++==>   SPEEDTEST TCPCache READ_REQ Time: [ %ld ] ms. \n\r", speedtest_stop());

						break;

					case 2:  //WRITE OPERATION
						if (DEBUG == 1)  speedtest_start();
						//pthread_mutex_lock(&mutex); // block mutex 
						//if (strlen (tst)< 30 ) printf ("tst[%s]\n\r",tst);
						if(Write_operation(tst) > 0) { //if recived siggnals founded and writed into server
							//pthread_mutex_unlock(&mutex); //unlock mutex
							write(sock, mesOk, strlen(mesOk));
							memset(client_message, 0, mess_length);
						} else {
							write(sock, mesErr, strlen(mesErr));			
							memset(client_message, 0, mess_length); //erase the buffer						 
						}
						break;

					default: //Error section
						printf("not read and not write, may be Error frame \n\r");
						printf("\n >>>>>>>> Unformatted Client Message -1!!!! \n\r");
						write(sock, mesBad, strlen(mesBad));
						memset(client_message, 0, mess_length);
				}  //end switch                                                                                                                                                   
			} //end while

			if(read_size == 0)    
			{   
				close (sock);
				client_sockets[i] = 0;
				//close(*arg->nSock);
			}
			else if(read_size == -1)
			{
				perror("SERVER: Recv failed");
			}
		}
	}

	return 0;
}
