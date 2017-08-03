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

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS               0
#define FAIL_ERROR			  1
#define NUM_THREADS 		  4
#define MAX_SIZE			100
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
// ********************************** Массив сигналов **********************************
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

void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
	someArgs_t args;
	int status,status_addr;
	char sepDotComma [10] = ";";
	char sepComma [10] = ",";
	char *iStr;
	char *pSB;
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
		
		args.nSock = new_sock;
		args.pSignal = Signals;
        if( pthread_create( &sniffer_thread, NULL,  connection_handler, (void*) &args) < 0)
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
		//printf("SERVER: Joined with address: %i\n", status_addr);
		//printf("from thread: %s", args.msg);
		printf("Last client by: %s \n", args.hello);
	//********************* Parse returned signal strings; (I)
		if( strstr(args.hello, "~core") != NULL ){
			iStr = strtok(args.msg,sepDotComma);
			int sigPos=0;
			while (iStr != NULL)
			{
				printf("%s \n", iStr);
				strcpy( Signals[sigPos].SrcString, iStr );//(*ptr_Signal)
				//*ptr_Signal++;
				iStr = strtok (NULL,sepDotComma);
				sigPos++;
			}				
	//********************* Parse returned signal strings; (II)
			char *iPos;		
			int x;		
			int z;
			for(x=0;x<MAX_SIZE;x++){
				if( strlen(Signals[x].SrcString) > 0 ){
					iPos = strtok(Signals[x].SrcString,sepComma);
					z=0;
					while(iPos != NULL){
						if(z==0){ strcpy(Signals[x].Name, iPos);}
						if(z==1){ Signals[x].Value[0] = atoi(iPos);}					
						z++;
						iPos = strtok(NULL,sepComma);
					}
				}
			}
	//********************* Print records from array of structure's (I)		
			int y;
			for(y=0;y<MAX_SIZE;y++){
				if( strlen(Signals[y].SrcString) > 0 ){
				  printf("%i: Signal Vame: %s, Signal Value: %i \n",y, Signals[y].Name, Signals[y].Value[0]);
				}
			}
		}
		if( strstr(args.hello, "~web") != NULL ){
			printf("\nOther client request: %s\n", "Try of web-request handler!" );
		}
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
 *	формируем ответ веб-клиенту
 */
char * sendSignals(const char * message){
    time_t now;
	char smallBuff[128];
    char timebuf[128];
	
	char strBuff[10000];
	char *pStrBuf = strBuff;
	char *pSmallBuff;

   //Переменная для системного времени
   long int s_time;
   //Указатель на структуру с локальным временем
   struct tm *m_time;

   //Считываем системное время
   s_time = time (NULL);
   //Преобразуем системное время в локальное
   m_time = localtime (&s_time);   
   strftime( timebuf, sizeof(timebuf), RFC1123FMT, m_time );
   now = time( (time_t*) 0 );
	// calc length of message
	size_t len = strlen(message) + 1;
	ItoA(len,smallBuff);   
	// pSmallBuff = itoa(len,smallBuff,10);
	/*****************************************************************/
	strcpy(strBuff,"HTTP/1.1 200 OK\015\012");
	strcat(strBuff,"Server: Sparc/0.0.1\015\012");
	strcat(strBuff, "Date: ");
	strcat(strBuff, timebuf);
	strcat(strBuff, "\015\012");
	strcat(strBuff, "Content-Type: text/html");
    strcat(strBuff, "\015\012");
	strcat(strBuff, "Content-Length:");
	strcat(strBuff, smallBuff );
    strcat(strBuff, "\015\012");
	strcat(strBuff, "Connection: Keep-Alive");
	strcat(strBuff, "\015\012");
	strcat(strBuff, "Keep-Alive: timeout=1, max=2");
	strcat(strBuff, "\015\012");
	strcat(strBuff, "\015\012");
	strcat(strBuff, message);
	strcat(strBuff, "\015\012");
	return pStrBuf;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *args)
{
	// from struct
	someArgs_t *arg = (someArgs_t*) args;
    //Get the socket descriptor
    int sock = *(int*)arg->nSock;
	//
	int read_size;
    char *mesOk, *mesNo , *messHello, client_message[2000], signalsBuffer[2000];
	//char header404[2000];
	// указатели вхождения метки в массиве посылки + общий указатель разделителя сигналов;
    char *iStr1, *iStr2, *iStr; 
	char *cmp1 = "start_set_signals";
	char *cmp2 = "end_set_signals";
	char *cmp3 = "GET / HTTP/1.1";
	char *cmp4 = "POST / HTTP/1.1";
	char *pCM = client_message;
	
	char httpOk[10000];
	char *pHttpOk = httpOk;

	char *pWeb = "~web";
	int posStartFrame =0;
	int posEndFrame=0;
	int cntFrame=0;
	char *pSB = signalsBuffer;
    mesOk = "Ok!\n";
	mesNo = "Ooh!\n";
	char a[4096];
	char dig[128];	
    //write(sock , messHello , strlen(messHello));
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0) ) > 0 )
    {
		int mess_length = sizeof(client_message) / sizeof(client_message[0]);
		printf("client_message: %s\n", client_message);
        /********  Команды от "Ядра Логики" - Modbus_Master_RTU  ***********************/
		iStr1 = strstr(pCM,cmp1); // команда CoreLogic start_set_signals;
		iStr2 = strstr(pCM,cmp2); // команда CoreLogic end_set_signals;
		if( (iStr1 != NULL) && (iStr2 != NULL) ){
			posStartFrame = (iStr1-pCM+1)+strlen(cmp1); // test value = 18;
			posEndFrame = iStr2-pCM -1; // test value = 65;
			cntFrame = posEndFrame - posStartFrame;
			char *subStringetBuff = substr(pSB,pCM,posStartFrame,cntFrame);
			arg->msg = subStringetBuff;
			arg->hello = "~core";
			write(sock, mesOk, strlen(mesOk));
			memset(client_message, 0, mess_length);
			close(*arg->nSock);
			free((int*)arg->nSock);
			pthread_exit(0);			
		}
		/********** Команды от "Веб-клиента" - Панель **********************************/
		iStr = strstr(pCM, cmp3); //если пришел запрос GET
		if( (iStr != NULL) ){
			size_t xx=0;
			size_t cnt=0;
			for(cnt=0; cnt<MAX_SIZE; cnt++)
			{
				if( strstr(arg->pSignal[xx].Name,"."))
				{
					strcat(a,"\t");
					strcat(a,arg->pSignal[xx].Name);
					strcat(a,"\t\t\tvalue: ");
					ItoA(arg->pSignal[xx].Value[0],dig);
					strcat(a,dig);
					strcat(a,"\015\012");					
					xx++;
				}
			}
			printf("\t__test__: \n~~~\n%s\n~~~\n", a);
			if(strlen(a)>10){
				pHttpOk = sendSignals(a);
				write( sock, pHttpOk, strlen(pHttpOk) );				
				arg->hello = "~web";
			}else{
				pHttpOk = sendSignals("Signal Matrix empty");
				write( sock, pHttpOk, strlen(pHttpOk) );				
				arg->hello = "~web";
			}

			printf("Send to Web-client ok headers:\n%s\n", pHttpOk);
			close(*arg->nSock);
			free((int*)arg->nSock);
			arg->msg = pWeb;
			memset(a, 0, sizeof(a)/sizeof(a[0]));
			memset(dig, 0, sizeof(dig)/sizeof(dig[0]));
			memset(client_message, 0, mess_length);
			pthread_exit(0);
		}
		
		/*******************************************************************************/
		if( (iStr1 == NULL) && (iStr2 == NULL) && (iStr == NULL) ){
			printf("\nUnformatted Client Message\n");
			arg->msg = "~~~";
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
