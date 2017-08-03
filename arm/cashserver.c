/*
     Cash TCP Server for data exchange between Logic_Core, Panel_Keyboard, Web-client;
	***********************************************************************************
	Created in May, 2017. Author: "Zeromnimus",zerom@sibmail.com; "'SPARC' Company"(c).
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<string.h> // strstr
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include<time.h> // for time_t

#include "signals.h"


#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS               0
#define FAIL_ERROR			  1
#define NUM_THREADS 		  4
#define MAX_SIZE			100
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
// ********************************** Массив сигналов **********************************

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
    int *nSock;
} Discrete_Signals_t;
                        
Discrete_Signals_t args[MAX_Signals]; // Create array of Tasks to control signals by threads
                         
                         





/*
struct signal {
	char Name[100];	//Name signal
	char SrcString[100]; // For parsing!
	int Value[2];	//value 2 int
	int Off;	//Signal not used if OFF = 1;
	int Ex_HW;	//Execution Hardware mark PcProc =1; Panel 43=2; Panel 10=3; soft_class
	int Ex_SF;	//Execution Software mark ModBusMaster_RTU =1 Modbus_Master_TCP=2 CoreSignal=100
};
struct signal Signals[MAX_SIZE];
struct signal *ptr_Signal;

// ********************* Упаковка аргументов в структуру для передачи из функции потока
typedef struct someArgs_tag {
    int id;
    char *msg;
	char *hello;
    int out;
	int *nSock;
	struct signal *pSignal;	
} someArgs_t;
*/

void *connection_handler(void *);
 
 
 
//**************************** MAIN SECTION ************************************************** 
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
	//someArgs_t args;
	//Discrete_Signals_t args;
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
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
		
		// структура аргументов для записи сокета и результата обработки
		
		args[0].nSock = new_sock;
		//args.pSignal = Signals;
		args[0].SA_ptr = Signal_Array;
        if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) &args[0]) < 0)
        {
            perror("SERVER: Could not create thread");
            return (ERROR_CREATE_THREAD);
        }
        puts("\nSERVER: Handler assigned"); 
        //Now join the thread , so that we dont terminate before the thread
        int status = pthread_join(sniffer_thread, (void**)&status_addr);
        if (status != SUCCESS) {
            printf("SERVER: Main error: can't join thread, status = %d\n", status);
            exit(ERROR_JOIN_THREAD);
        }
		
		//printf("Last client by: %s \n", args.hello);
		
    }
	
	
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
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

/*
 * This will handle connection for each client
 * */
void* connection_handler(void *args)
{
// from struct
//	someArgs_t *arg = (someArgs_t*) args;
	Discrete_Signals_t *arg = (Discrete_Signals_t*) args;
	
int n=0;
  for (n=0; n < MAX_Signals; n++)
      {
        printf("THREAD Found Signal Name:[ %s ] IN Signal_Array\n\n", arg->SA_ptr[n].Name);
        if (strstr(arg->SA_ptr[n].Name ,"." ) != NULL ) //if found "." in field Name
            {
               printf("THREAD Found Signal Name:[ %s ] IN Signal_Array\n\n", arg->SA_ptr[n].Name);
                    
            }
        }
                                                   
    //Get the socket descriptor
    int sock = *(int*)arg->nSock;
	//
	int read_size;
    char *mesOk, *mesNo, *mesErr, *messHello, client_message[10000], signalsBuffer[10000];
	//char header404[2000];
	// указатели вхождения метки в массиве посылки + общий указатель разделителя сигналов;
    char *iStr1, *iStr2, *iStr, *iCmd1, *iCmd2, *iCmdEnd; 
	char *cmd_read_signal = "signal_read:";
	char *cmd_write_signal = "signal_write:";
	char *cmd_end = ";";
	char *cmp1 = "start_set_signals";
	char *cmp2 = "end_set_signals";

	char *pCM = client_message;
	
	char *istr,ival;

	int posStartFrame =0;
	int posEndFrame=0;
	int cntFrame=0;
	char *pSB = signalsBuffer;
	mesErr="-1";
	mesOk = "Ok!";
	mesNo = "Ooh!\n";
	char a[4096];
	char dig[128];	
    //write(sock , messHello , strlen(messHello));
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 10000 , 0) ) > 0 )
    {
		int mess_length = sizeof(client_message) / sizeof(client_message[0]);
		printf("client_message: [%s] \r\n", client_message);
		
        /********  Команды от "Ядра Логики" - Modbus_Master_RTU  ***********************/
        //Проверка наличия в строке команд и признака конца команды
		iStr1 = strstr(pCM,cmp1); // команда CoreLogic start_set_signals;
		iStr2 = strstr(pCM,cmp2); // команда CoreLogic end_set_signals;
		
		iCmd1 = strstr (pCM,cmd_read_signal); // check for input cmd "signal_read"
		iCmd2 = strstr (pCM,cmd_write_signal); // check for input cmd "signal_write"
		iCmdEnd = strstr (pCM,cmd_end); // check for input cmd end ";"
	//
//		if( (iCmd1 != NULL) && (iCmdEnd != NULL) ){ // cmd read_signal
		if( iCmd1 != NULL ){ // cmd read_signal
		    printf("recived >CMD read_signal< \n\r");
			size_t xx=0;
			size_t cnt=0;
			istr =strtok(client_message,":");
			istr = strtok (NULL,":");
			
			// Выделение последующих частей
			/*
			while (istr != NULL)
			     {
			                // Вывод очередной выделенной части
			                   printf ("EXPLODE: [ %s ]\n\r",istr);
			                // Выделение очередной части строки
			                istr = strtok (NULL,":");
			     }
			     */
			printf ("NAME: [ %s ]\n\r",istr);
			int found=0;  
			char result[30000];  //buffer for response
			strcpy (result,""); //erase buffer
			for(cnt=0; cnt <  MAX_Signals; cnt++)
			{
				if( strstr(arg->SA_ptr[cnt].Name, istr))
				{       
				        strcpy(packed_txt_string,""); //erase buffer
				        sSerial_by_num(cnt); //serialize to packet by number of signals				        
				        strcat (result,packed_txt_string);
				        printf ("[ %s ]\n\r",packed_txt_string);
				        
					//strcat (result,arg->SA_ptr[cnt].Name);
					//strcat (result," value:");
					/*
					printf ("[%i] Signal Name: [%s]\t",cnt,arg->SA_ptr[cnt].Name);
					printf (" mb_id:      [%i] \t ",arg->SA_ptr[cnt].MB_Id);
					printf (" mb_reg_num: [%i] \t ",arg->SA_ptr[cnt].MB_Reg_Num);
					printf (" bit_pos:    [%i] \t ",arg->SA_ptr[cnt].Bit_Pos);
					printf (" Value0:     [%i] \t ",arg->SA_ptr[cnt].Value[0]);
					printf (" Value1:     [%i] ",arg->SA_ptr[cnt].Value[1]);
					printf (" Off:        [%i] ",arg->SA_ptr[cnt].Off);
					printf (" ExState:    [%i] \n\r",arg->SA_ptr[cnt].ExState);
					
					ItoA (arg->SA_ptr[cnt].Value[0],dig);
					strcat (result,dig);
					*/
					/*
					strcat(result,":");
					ItoA (arg->SA_ptr[cnt].MB_Id,dig);
					strcat (result,dig);
					
					strcat(result,":");
					ItoA (arg->SA_ptr[cnt].MB_Reg_Num, dig);
					strcat (result,dig);
					
					strcat(result,":");
					ItoA(arg->SA_ptr[cnt].Bit_Pos, dig);
					strcat (result,dig);
					
					strcat(result,":");
					strcat(result,arg->SA_ptr[cnt].Off  );
					
					strcat(result,":");
					strcat(result,arg->SA_ptr[cnt].ExState  );
					*/
					
					//strcat (result,"; ");
					xx++;
					found++;
				}
			}
			
			if (found == 0) {
			printf("Signal not found \n\r");
			
			//arg->hello = "~core";
			write(sock, mesErr, strlen(mesErr));
			}

			if (found > 0) {
			printf("Signal  found [%i]! \n\r",found);
			
			//arg->hello = "~core";
			strcat (result,mesOk); //add Ok to end
			write(sock, result, strlen(result)); //send packet to client
			}
			memset(client_message, 0, mess_length);
			close(*arg->nSock);
			free((int*)arg->nSock);
			pthread_exit(0);		
		    
		}
		
		if( iCmd2 != NULL ){ // cmd write_signal
		    char digit[5];
		    char sname [55];
		    int val;
		    printf("recived >CMD write_signal< \n\r");
			size_t xx=0;
			size_t cnt=0;
			istr =strtok(client_message,":");
			istr = strtok (NULL,":");			
			printf ("EXPLODE NAME: [ %s ]\n\r",istr);
			strcpy (sname,istr);
			
			istr = strtok (NULL,":");						
			printf ("EXPLODE digital: [ %s ]\n\r",istr);
			strcpy(digit,istr);
			printf("Before convertation: [%s]\n\r", digit);
			int found=0;         
			printf("looking for Signal: [%s] \n\r",sname);
			val =  atoi(digit);
			printf("Value AtoI to write: [%i] \n\r",val);
			for(cnt=0; cnt<MAX_Signals; cnt++)
			{

				if( strstr(arg->SA_ptr[cnt].Name,sname) != NULL )
				{				
				printf("SignalName:[%s]",arg->SA_ptr[cnt].Name);
				arg->SA_ptr[cnt].Value[0] =val;
				printf(" AtoI writed value [%i]\n\r",arg->SA_ptr[cnt].Value[0]);
				xx++;
				found++;
				}
			}
			if (found == 0) {
			printf("Signal not found \n\r");
			
			//arg->hello = "~core";
			write(sock, mesErr, strlen(mesErr));
			}

			if (found > 0) {
			printf("Founded signals [%i]\n\r",found);
			
			//arg->hello = "~core";
			write(sock, mesOk, strlen(mesOk));
			}
			memset(client_message, 0, mess_length);
			close(*arg->nSock);
			free((int*)arg->nSock);
			pthread_exit(0);		
		    
		}
		
		
		/****************************** ERROR CMD SECTION ********************************************/
		
		if( (iCmd1 == NULL) && (iCmd2 == NULL)  ){
			printf("\nUnformatted Client Message\n");
			//arg->msg = "~~~";
			memset(client_message, 0, mess_length);
			close(*arg->nSock);
			free((int*)arg->nSock);
			pthread_exit(0);			
		}		
		
    }
     
    if(read_size == 0)
    {
        puts("SERVER: Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("SERVER: Recv failed");
    }
	close(*arg->nSock);         
    //Free the socket pointer
    free((int*)arg->nSock);
     
    return 0;
}
