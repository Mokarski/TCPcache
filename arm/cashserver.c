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
#include<pthread.h>   //for threading , link with lpthread
#include<time.h>      // for time_t
#include<errno.h>     // for print errors

#define DEBUG 0       // 0 -not debug  1- debug
#include "signals.h"
#include "network.h"
#include "speedtest.h"

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS               0
#define FAIL_ERROR			  1
#define NUM_THREADS 		  4
#define MAX_SIZE			100
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


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
                         


        void * getglobalsignals() { return (void*)&args; }




void *connection_handler(void *);
 
 
 
//**************************** MAIN SECTION ************************************************** 
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
	//someArgs_t args;

	args.SA_ptr = Signal_Array;
	int status,status_addr;
	char sepDotComma [10] = ";";
	char sepComma [10] = ",";
	char *iStr;
	char *pSB;
	
    //Load signals
    init_signals_list();
    signals_file_load();
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
    server.sin_port = htons( 8888 );
     
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
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {   printf("\n\r ***TCPSERVER: \n\r");
        puts("Connection accepted");
        printf("(MAIN While) CLient socket ID[#%i] \n\r",client_sock); 
		
		
         pthread_t sniffer_thread;
	 pthread_attr_t threadAttr;      // Set up detached thread attributes
         pthread_attr_init(&threadAttr);
         pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
         
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
		
        //if( pthread_create( &sniffer_thread, &threadAttr,  connection_handler, (void*) &args) < 0) //for detached thread
        // if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) &args) < 0)
        if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) client_sock) < 0)
        {
            perror("SERVER: Could not create thread");
            return (ERROR_CREATE_THREAD);
        }
        puts("\nSERVER: Handler assigned"); 
		// Very HARD problem with thread resorces and thread termination
		
        //Now join the thread , so that we dont terminate before the thread
        int status = pthread_join(sniffer_thread, (void**)&status_addr);
        if (status != SUCCESS) {
            printf("SERVER: Main error: can't join thread, status = %d\n", status);
            exit(ERROR_JOIN_THREAD);
        }
		
		
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



/*
 * This will handle connection for each client
 * */
void* connection_handler (void *args)
{

// from struct
//		

	Discrete_Signals_t *arg = (Discrete_Signals_t * ) getglobalsignals();
        int n=0;

    //Get the socket descriptor
    //int sock = arg->nSock; //problem one id to many threads
    int sock =(int)args; // client_sock
    printf(">>>> THREAD Socket ID[#%i] \n\r",sock);
	//
	int read_size;
    char *mesOk, *mesNo, *mesErr, *mesBad,  client_message[MAX_MESS], signalsBuffer[MAX_MESS];
    char *iStr1, *iStr2, *iStr, *iCmd1, *iCmd2, *iCmdEnd; 
	char *cmd_end = ";";
	char *pCM = client_message;
	char *istr,ival;
	mesOk = "Ok!";	
	mesBad = "NOcmd!";
	mesErr = "Err!";
	char a[4096];
	char dig[128];	

	char tst[MAX_MESS];
	int rd_wr=0;
        int found=0;  //founded signals counter
        int iterration=0;
    while( (read_size = recv(sock , client_message , MAX_MESS , 0) ) > 0 )
    {
      if (DEBUG == 1) iterration++;
		int mess_length = sizeof(client_message) / sizeof(client_message[0]);
		 if (DEBUG == 1) printf("\n\r {Cycle %i} \n\r [SRV received: %i bytes] client_message: [%s]\r\n",iterration,read_size ,client_message);

		
	//********************************* COMMAND SELECTION AND EXECUTION ******************************/
		rd_wr = frame_unpack(client_message,tst);
		if (rd_wr < 0) {
		printf ("FRAME_UNPACK ERROR: %i \n\r",rd_wr);
		return 0; 
		} else {
  		         if (DEBUG == 1) printf("[FRAMEUNPACK: rd_wr[%i]]   Unpacked: [%s] \r\n", rd_wr, tst);	   
  		         
  		         
  		         }
	   
       //********************************** READ **********************************************************
       //if( iCmd1 != NULL ){ // cmd read_signal
       if( rd_wr == 1 ){ // cmd read_signal
                //pthread_mutex_lock(&mutex); // block mutex 
                int rc;
                if ( (rc = pthread_mutex_trylock(&mutex)) == EBUSY ) {
                        printf("Mutex is already locked by another process.\nLet's lock mutex using pthread_mutex_lock().\n");
                        pthread_mutex_lock (&mutex);
                        } else if ( rc == 0 ) {
                                                printf("Mutex lock by Read operation!\n");
                                                } else { 
                                                         printf("Error: %d\n", rc);                                                        
                                                        }
            
                
		speedtest_start();
		char sep_comma[4]=";";
		    found=0;  
		    printf("[recived >CMD READ] read_signal \n\r");
		    printf("THREAD Socket ID[#%i]\n\r",sock);
			size_t xx=0;
			size_t cnt=0;
			printf ("SERVER: recive from client: [%s]\n\r",tst);
			istr = strtok(tst,sep_comma);
			if (istr != NULL){
			    printf ("SERVER: GET the NAME [%s]\n\r",istr);
			    }
			/*
			if (istr != NULL){
			    istr = strtok (NULL,";"); //mask or signal name
			   }
			
			if (istr != NULL) printf ("NAME: [ %s ]\n\r",istr);
			*/
			char result[MAX_MESS];  //buffer for response 50 000 bytes
			char result2[MAX_MESS];  //buffer for response 50 000 bytes
			strcpy (result,""); //erase buffer
			for(cnt=0; cnt <  MAX_Signals; cnt++)
			{
				if( strstr(arg->SA_ptr[cnt].Name, istr))
				{
				 //      printf ("[%i] Signal Name: [%s]\t",cnt,arg->SA_ptr[cnt].Name); //debug
				       if ( arg->SA_ptr[cnt].Value[1] > 0 ){
				           if (DEBUG == 1)    printf ("Socket[%i]|  READ: Signal > 0 [ Name: [%s]  Value: {%i} ExState: [%i] ] \n\r",sock,arg->SA_ptr[cnt].Name,arg->SA_ptr[cnt].Value[1],arg->SA_ptr[cnt].ExState); //debug
				          }
				        //arg->SA_ptr[cnt].ExState=0; // Flag ExState turn off 
				        
				        strcpy(packed_txt_string,""); //erase buffer
				        sSerial_by_num(cnt); //serialize to packet by number of signals				        
				        strcat (result,packed_txt_string);
				        //printf ("[ %s ]\n\r",packed_txt_string); //DEBUG

					xx++;
					found++;
				}
			}
			pthread_mutex_unlock(&mutex); //unlock mutex
			if (found == 0) {
			printf("CMD_READ: Signal not found/ Close socket \n\r");
			
			//arg->hello = "~core";
			//mesErr = "Err! NotFound \n\r";
			write(sock, mesErr, strlen(mesErr));
			
			memset(client_message, 0, mess_length);
			//close(*arg->nSock);
			//free((int*)arg->nSock);
			//pthread_exit(0);		
			}

			if (found > 0) { //section to send client finded signals
			printf("Signals READ  found [%i]! \n\r",found);
						
			//strcat (result,mesOk); //add Ok to end
			//printf ("BUF to SEND:[%s] \n \n \r",result); //debug
			frame_pack("Ok!",result,result2); //pack all info to FRAME
			if (DEBUG == 1) printf ("result2: {%s}",result2);
			//write(sock, result, strlen(result)); //send packet to client			
			int msg_len = strlen(result2);
			int write_ok;
			write_ok = write(sock, result2, msg_len ); //send packet to client			
			if (DEBUG == 1) printf("\n\rTry to Write, write_ok state: [%i] \n\r",write_ok);
			if (write_ok < 0){
			    
                             printf("Error description is : %s\n",strerror(errno));			    

			    }
			strcpy (client_message,"");
			memset(client_message, 0, mess_length);
			}
			/*
			memset(client_message, 0, mess_length);
			close(*arg->nSock);
			free((int*)arg->nSock);
			pthread_exit(0);		
		        */
		         printf(" ++++++++++++++++++++++++==>   SPEEDTEST TCPCache READ_REQ Time: [ %ld ] ms. \n\r", speedtest_stop());
		    
		}
		//*********************************** WRITE ************************************************************
		//if( iCmd2 != NULL ){ // cmd write_signal
		 if( rd_wr == 2 ) {
		    //pthread_mutex_lock(&mutex); // block mutex 
		    int rc;
		                    if ( (rc = pthread_mutex_trylock(&mutex)) == EBUSY ) {
                                          printf("Mutex is already locked by another process.\nLet's lock mutex using pthread_mutex_lock().\n");
                                          pthread_mutex_lock (&mutex);
                                         } else if ( rc == 0 ) {
                                                printf("Mutex lock by Write operation!\n");
                                                } else { 
                                                         printf("Error: %d\n", rc);                                                        
                                                        }
		    
		    speedtest_start();
		    found=0;          //flag how many founded signals
		    char digit[5];    //buffer Value of signal as CHAR
		    char sname [100]; //buffer for temp store signal NAME
		    char buf_signals[MAX_Signals][150]; //array of MAX_signal elements AND 150 characters each
		    int t=0;		        
		    int val;
		    
		    /*
		          for (t=0; t < MAX_Signals; t++){
		               strcpy(buf_signals[t],""); //clear buffer
		              }
		    */        
		        printf("[recived >CMD WRITE] write_signal \n\r");
		        //printf("THREAD Socket ID[#%i]\n\r",sock);
			size_t xx=0;
			size_t cnt=0;			
			int sn=1; //number of signals started from 1, because before while we get 1 signal
			
			if (DEBUG == 1) printf ("SERVER: recive from client sock_id[%i]: [%s]\n\r",sock,tst);
			istr =strtok(tst,";");			            
			if ( istr != NULL ){
			    strcpy (buf_signals[0],istr);
			   } else {printf ("Not found delimiter [;]\n\r");}
			   
			//istr = strtok (NULL,";");			
			//printf ("EXPLODE NAME: [ %s ]\n\r",istr);
			
			while ( istr != NULL ){
			        /*if (istr == NULL) // defend from sigfault
			        {
			         printf("End write list\n\r");
			         return ;
			        }*/
				istr =strtok(NULL,";");
				/*
				if ( istr != NULL ) // defend from sigfault
			        {
			         strcpy (buf_signals[sn],istr);
				 printf ("[#%i] NAME: [%s] \n\r",sn,buf_signals[sn]);
			         }
			         */
			         if (istr == NULL) // defend from sigfault
			        {
			         printf("Socket[%i]|  End write to cachebuf, at position [%i]\n\r",sock,sn);
			         // return ;
			         break;
			        } else {
			                //strcpy (buf_signals[sn],""); //erase buffer segment before copy new parameter
			    		strcpy (buf_signals[sn],istr);
					if (DEBUG == 1) printf ("Socket[%i]| explode ; to cachebuf [#%i]  NAME: [%s] \n\r",sock,sn,buf_signals[sn]); //debug
			               }
				
				sn++;
			   }
			
			if (DEBUG == 1) printf("Signal_counter sn=%i\n\r",sn);		
			         
			for(cnt=0; cnt < MAX_Signals; cnt++) //cycle for signals
			{ 
			    //if (DEBUG == 1) printf ("buf_signals[%s] -  SignalsName [%s]\n\r",buf_signals[cnt],arg->SA_ptr[cnt].Name); //debug
			    
			    int pr=0;
			    if ( strlen ( arg->SA_ptr[cnt].Name ) > 2 ) {  // test if signal name not empty
			             //printf ("Cyrrent SA: [%s] \n\r",arg->SA_ptr[cnt].Name);
			             
			         for (pr = 0; pr < sn; pr++){ //cycle for recived signals from client. number of recived signals = sn
			
			              //************************** !!!!!!!!!!!!!!!!!! **********************
			                //printf("[Total clientsignals#%i][#%i]Cyrrent client signal: [%s]\n\r",sn,pr,buf_signals[pr]);
			                //if (DEBUG == 1) printf ("StrStr: buf_signals[%s]  -  SignalsName [%s]\n\r",buf_signals[pr],arg->SA_ptr[cnt].Name); //debug
			                //pack_signal(cnt,tmpz);
			                //if (DEBUG == 1) printf ("StrStr: buf_signals[%s]  -  SignalsName [%s]\n\r",buf_signals[pr],tmpz); //debug
			                char tmpz[190];
			                char *istrName;
			                strcpy(tmpz,buf_signals[pr]); //tmp buf
			                //printf("tmpz %s\n\r",tmpz);
			                istrName =strtok(tmpz,":"); //extract name			            
			                if ( istrName != NULL ){
			                    //printf("istr %s \n\r",istrName);
			                    strcpy (tmpz,istrName);
			                    //printf("tmpz %s \n\r",tmpz);
			                    } else {
			                             printf ("Not found Name in buf_signals[:]\n\r");
			                             break;
			                             }
			                    
			                if ( strstr( arg->SA_ptr[cnt].Name,tmpz  ) != NULL ) { //if in buffer we find signal name			                   
			                    if (strcmp (arg->SA_ptr[cnt].Name, tmpz)==0) 
			                       { 
			                         if ( DEBUG ==1 ) printf(">>>CMPNAME SignalName=[%s]\n\r",arg->SA_ptr[cnt].Name);

			                    
			                         found++;
			                         int utest=0;
			                         if ( DEBUG == 1) printf("SignalName[%s] = RecivedName[%s]",arg->SA_ptr[cnt].Name,tmpz);
			                         if (DEBUG == 1) printf ("before unpack %s \n\r",buf_signals[pr]);			                    
			                         utest = unpack_signal(buf_signals[pr],pr); //unpack from field buffer to signal properties fields
			                       }
			                   }
			            
			          }
			          
			      
			      //pr++; // increment start index position of cycle for start next step exclude previus step!!!
			    }
			}
			
			pthread_mutex_unlock(&mutex); //unlock mutex
			
			if (found == 0) {
			printf("CMD_WRITE: Signal not found! [%i] \n\r",found);
			
			//arg->hello = "~core";
			//mesErr = "Err! WriteSignals Not FOund ";
			write(sock, mesErr, strlen(mesErr));			
			memset(client_message, 0, mess_length); //erase the buffer
			strcpy(client_message,""); //erase the buffer
			//close(*arg->nSock);
			//free((int*)arg->nSock);
			//pthread_exit(0);		
			}

			if (found > 0) {
			printf("Founded WRITE signals [%i]\n\r",found);
			//mesOk = "Ok!";
			//arg->hello = "~core";
			write(sock, mesOk, strlen(mesOk));
			memset(client_message, 0, mess_length);
			}
			
			/*
			memset(client_message, 0, mess_length);
			close(*arg->nSock);
			free((int*)arg->nSock);
			pthread_exit(0);		
		       */
		        printf(" ++++++++++++++++++++++++==>   SPEEDTEST TCPCache WRITE_REQ Time: [ %ld ] ms. \n\r", speedtest_stop());
		        
		}
	
		
		
		/****************************** ERROR CMD SECTION ********************************************/
		// At this place we have no command write or read occured
		//if(  (iCmd1 == NULL) && (iCmd2 == NULL)  ){
		 if( rd_wr == -1 ) {
			printf("\n >>>>>>>> Unformatted Client Message -1!!!! \n\r");
			//arg->msg = "~~~";
			write(sock, mesBad, strlen(mesBad));
			memset(client_message, 0, mess_length);
			strcpy(client_message,"");
			//close(*arg->nSock);
			//free((int*)arg->nSock);
			//pthread_exit(0);			
		}		
		
		if( rd_wr == -2 ) {
			printf("\n >>>>>>>> Unformatted Client Message -2!!!! \n\r");
			//arg->msg = "~~~";
			write(sock, mesBad, strlen(mesBad));
			memset(client_message, 0, mess_length);
			strcpy(client_message,"");
			//close(*arg->nSock);
			//free((int*)arg->nSock);
			//pthread_exit(0);			
		}
		
    }
     
    if(read_size == 0)    
    {   
        printf("ERROR^: recived size = 0 \n\r  THREAD Socket ID[#%i]\n\r",sock);
        //printf ("SERVER RECIVED MESSAGE: {%s} \n\r",client_message);
        puts("SERVER: Client disconnected / Read_ Size = 0");
        fflush(stdout);
        //close(*arg->nSock);
    }
    else if(read_size == -1)
    {
        perror("SERVER: Recv failed");
    }
	//close(*arg->nSock);         
	close (sock);
    //Free the socket pointer
    //printf(" ++++++++++++++++++++++++==>   SPEEDTEST  TCPCache write from client Time: [ %ld ] ms. \n\r", speedtest_stop());
     
    //free((int*)arg->nSock);
    //free (sock);
    return 0;
}
