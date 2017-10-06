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

#define DEBUG 3       // 0 -not debug  1,2,3- debug
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
#define PORT 8888
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
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {   printf("\n\r ***TCPSERVER: \n\r");
        puts("Connection accepted");
        printf("\n\r>>>>>    (MAIN While) CLient socket ID[#%i] \n\r",client_sock); 
		
		
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
		

        if( pthread_create( &sniffer_thread, &threadAttr,  connection_handler, (void*) client_sock) < 0)
        {
            perror("SERVER: Could not create thread");
            return (ERROR_CREATE_THREAD);
        }
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





//********************************** READ **********************************************
int Read_operation(char tst[MAX_MESS], char to_send[MAX_MESS]){
        	char *istr;                
		char sep_comma[4]=";";
		int found=0;  
		    printf("[recived >CMD READ] read_signal \n\r");		    
			size_t xx=0;
			size_t cnt=0;
			printf ("SERVER: recive from client: [%s]\n\r",tst);
			istr = strtok(tst,sep_comma);
			if (istr != NULL){
			    printf ("SERVER: GET the NAME [%s]\n\r",istr);
			    }

			char tmpz[150];
			//strcpy (result,""); //erase buffer
			for(cnt=0; cnt <  MAX_Signals; cnt++)
			{
				if( strstr(Signal_Array[cnt].Name, istr) !=NULL )
				{
				       if ( Signal_Array[cnt].Value[1] > 0 ){
				           if (DEBUG == 1)    printf ("  READ: Signal > 0 [ Name: [%s]  Value: {%i} ExState: [%i] ] \n\r",Signal_Array[cnt].Name,Signal_Array[cnt].Value[1],Signal_Array[cnt].ExState); //debug
				          }
				        pack_signal(cnt,tmpz);
				        strcat (to_send,tmpz);
					xx++;
					found++;
				}
			 if (DEBUG == 3) printf ("#%i  <<---- R-SignalsName [%s]  Val{%i} Ex{%i}  \n\r",cnt,Signal_Array[cnt].Name, Signal_Array[cnt].Value[1] ,Signal_Array[cnt].ExState); //debug
			}
 printf("Signals READ  found [%i]! \n\r",found);
return found; //founded signal counter
}
//**************************************************************************************


//********************************** WRITE *********************************************
int Write_operation (char tst[MAX_MESS]){
		    char *istr;
		    int found=0;          //flag how many founded signals
		    char digit[5];    //buffer Value of signal as CHAR
		    char sname [150]; //buffer for temp store signal NAME
		    char buf_signals[MAX_Signals][150]; //array of MAX_signal elements AND 150 characters each
		    int t=0;		        
		    int val;
		    size_t xx=0;
		    size_t cnt=0;					    
		    
		    printf("[recived >CMD WRITE] write_signal \n\r");
		    int sn=1; //number of signals started from 1, because before while we get 1 signal
			
			if (DEBUG == 1) printf ("SERVER: recive from client WRITE Req [%s]\n\r",tst);
			istr =strtok(tst,";");			            
			if ( istr != NULL ){
			    strcpy (buf_signals[0],istr);
			   } else {printf ("Not found delimiter [;]\n\r");}
			
			while ( istr != NULL ){
				istr =strtok(NULL,";");
			         if (istr == NULL) // defend from sigfault
			            {
			              printf(" End write to cachebuf, at position [%i]\n\r",sn);
			              break;
			             } else {			            
			    		      strcpy (buf_signals[sn],istr);
					      if (DEBUG == 1) printf ("Explode ; to cachebuf [#%i]  NAME: [%s] \n\r",sn,buf_signals[sn]); //debug
			                    }
				
				sn++;
			      }
			
			if (DEBUG == 1) printf("Signal_counter sn=%i\n\r",sn);		
			cnt=0;         
			for(cnt=0; cnt < MAX_Signals; cnt++) //cycle for signals
			{ 
					    
			    int pr=0;
			    if ( strlen ( Signal_Array[cnt].Name ) > 2 ) {  // test if signal name not empty
			             
			         for (pr = 0; pr < sn; pr++){ //cycle for recived signals from client. number of recived signals = sn
			
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
			                    
			                if ( strstr( Signal_Array[cnt].Name,tmpz  ) != NULL ) { //if in buffer we find signal name			                   
			                    if (strcmp (Signal_Array[cnt].Name, tmpz)==0) 
			                       { 
			                         if ( DEBUG ==1 ) printf(">>>CMPNAME SignalName=[%s]\n\r",Signal_Array[cnt].Name);
			                         found++;
			                         int utest=0;
			                         if ( DEBUG == 1) printf("SignalName[%s] = RecivedName[%s]",Signal_Array[cnt].Name,tmpz);
			                         if (DEBUG == 1) printf ("before unpack %s \n\r",buf_signals[pr]);			                    
			                         utest = unpack_signal(buf_signals[pr],pr); //unpack from field buffer to signal properties fields
			                       }
			                   }
			            
			          }
			          
			      
			      //pr++; // increment start index position of cycle for start next step exclude previus step!!!
			    }
			 //if (DEBUG == 3) printf ("#%i thread--- >> WR-SignalsName [%s] Val{%i} Ex{%i} \n\r",cnt,Signal_Array[cnt].Name, Signal_Array[cnt].Value[1] ,Signal_Array[cnt].ExState); //debug
			 if (DEBUG == 3) printf ("#%i SA--- >> WR-SignalsName [%s] Val{%i} Ex{%i} \n\r",cnt,Signal_Array[cnt].Name, Signal_Array[cnt].Value[1] ,Signal_Array[cnt].ExState); //debug

			 if (DEBUG == 2){
			    if (Signal_Array[cnt].ExState == 2){
			        //printf ("#%i IN THRead --- >> WR-SignalsName [%s] Val{%i} Ex{%i} \n\r",cnt,arg->SA_ptr[cnt].Name, arg->SA_ptr[cnt].Value[1] ,arg->SA_ptr[cnt].ExState); //debug
			        printf ("#%i SA--- >> WR-SignalsName [%s] Val{%i} Ex{%i} \n\r",cnt,Signal_Array[cnt].Name, Signal_Array[cnt].Value[1] ,Signal_Array[cnt].ExState); //debug
			        }
			    
			   }

			}
 printf("Signals WRITE  found [%i]! \n\r",found);
return found; //return number of written signals
}
//**************************************************************************************

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
    char *mesOk, *mesNo, *mesErr, *mesBad, *mesUnp;
    char  client_message[MAX_MESS];
    char  client_message_write[MAX_MESS];    
    char  signalsBuffer[MAX_MESS];
    char *cmd_end = ";";
    mesOk = "Ok!";	
    mesBad = "NOcmd!";
    mesErr = "Err!";
    mesUnp = "Err! unpack Farme!";
//    char a[4096];
    char dig[128];	
    //char packed_txt_string[40000];
    char tst[MAX_MESS];
    int rd_wr=0;
//    int found=0;  //founded signals counter
    int iterration=0;
    while( (read_size = recv(sock , client_message , MAX_MESS , 0) ) > 0 )
    {
      if (DEBUG == 1) iterration++;
		int mess_length = sizeof(client_message) / sizeof(client_message[0]);
		 if (DEBUG == 1) printf("\n\r {inTHREAD_sock[%i] Cycle %i} \n\r [SRV received: %i bytes] client_message_read: [%s]\r\n",sock,iterration,read_size ,client_message);
		 if (strlen(client_message) < 30) printf(">>>Recived from client[%s]\n\r",client_message);

		
	//********************************* COMMAND SELECTION AND EXECUTION ******************************/
	    pthread_mutex_lock(&mutex); // block mutex 
	        strcpy(tst,""); //erase buffer
		rd_wr = frame_unpack(client_message,tst);
	    pthread_mutex_unlock(&mutex); //unlock mutex
	    
		if (rd_wr < 0) {
		    printf ("\n ===============================================================\n");
		    printf ("----------------------> FRAME_UNPACK ERROR!!!!!: %i \n\r",rd_wr);
		    if (DEBUG > 0) printf("\n\r {Cycle %i} \n\r [SRV received: %i bytes] client_message: [%s]\r\n",iterration,read_size ,client_message);
		    printf ("\n ===============================================================\n");
		    write(sock, mesUnp, strlen(mesUnp));
		    close (sock); // close socket before exit
		    printf("Drop the socet and close connection. \n\r");
		    return 0; 
		   } else {
  		          if (DEBUG == 1) printf("[FRAMEUNPACK: rd_wr[%i]]   Unpacked: [%s] \r\n", rd_wr, tst);	   
  		         }
  		         
  		         
  		   
		   switch (rd_wr){
  		                   case 1:  //READ OPERATION
  		                	 speedtest_start();
  		                	 pthread_mutex_lock(&mutex); // block mutex 
  		                	 char result[MAX_MESS];  //buffer for response 50 000 bytes
					 char result2[MAX_MESS];  //buffer for response 50 000 bytes
					    strcpy(client_message,"");
  		                	 if (Read_operation(tst,client_message) > 0) { //if founded requested name of signals
  		                	   
			                    
					     frame_pack("Ok!",client_message,result2); //pack all info to FRAME
					     if (DEBUG == 1) printf ("result2: {%s}",result2);
					     int msg_len = strlen(result2);
					     int write_ok;
					     pthread_mutex_unlock(&mutex); //unlock mutex
					     
					     write_ok = write(sock, result2, msg_len ); //send packet to client			
					     if (DEBUG == 1) printf("\n\rTry to Write,  Sendded bytes: [%i] \n\r",write_ok);
					     
					     if (write_ok < 0){
                            			 printf("TCP SEND Error description is : %s\n",strerror(errno));			    
						}
						
					     strcpy (client_message,"");
					     memset (client_message, 0, mess_length);
					     
					  } else {  // if signals not found
					           printf("CMD_READ: Signal not found \n\r");
						   write(sock, mesErr, strlen(mesErr));			
						   memset(client_message, 0, mess_length);
  		                	         }
  		                	 printf(" ++++++++++++++++++++++++==>   SPEEDTEST TCPCache READ_REQ Time: [ %ld ] ms. \n\r", speedtest_stop());

  		                   break;
  		                   
  		                   
  		                   
  		                   
  		                   case 2:  //WRITE OPERATION
  		                         speedtest_start();
  		                 	 pthread_mutex_lock(&mutex); // block mutex 
  		                 	 if (strlen (tst)< 30 ) printf ("tst[%s]\n\r",tst);
  		                 	 if (Write_operation(tst) > 0) { //if recived siggnals founded and writed into server
			                     write(sock, mesOk, strlen(mesOk));
			                     memset(client_message, 0, mess_length);
  		                 	 
  		                 	 } else {
						 write(sock, mesErr, strlen(mesErr));			
						 memset(client_message, 0, mess_length); //erase the buffer
						 strcpy(client_message,""); //erase the buffer
  		                 	         }
  		                 	 
  		                 	 printf(" ++++++++++++++++++++++++==>   SPEEDTEST TCPCache WRITE_REQ Time: [ %ld ] ms. \n\r", speedtest_stop());
					 pthread_mutex_unlock(&mutex); //unlock mutex
  		                   break;
  		                   
  		                   default: //Error section
  		                            printf("not read and not write, may be Error frame \n\r");
  		                            printf("\n >>>>>>>> Unformatted Client Message -1!!!! \n\r");
					    write(sock, mesBad, strlen(mesBad));
					    memset(client_message, 0, mess_length);
					    strcpy(client_message,"");
					    memset(client_message_write, 0, mess_length);
					    strcpy(client_message_write,"");
					    
				
  		   }  //end switch                                                                                                                                                   
	   



		        
	} //end while
	
		
		
		
 
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
    close (sock);
return 0;
}
