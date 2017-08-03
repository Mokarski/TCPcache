 //    C socket server example, handles multiple clients using threads
 // WARNING  
 // gcc -pthread -o term term.c
 //

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
 
#define PORT 5000  // порт, на который будут приходить соединения
#define BUFFER 2000  // input buffer size in bytes
#define MAX_Signals 256 //number of signals

typedef struct Discrete_Signals { // store one  signal state
char Input_Signal[100];        //searched signal name
char Reaction_Signal[100];     //reaction signal name
int adc_val;                   //control this state on ADC
int result;                    // returned value
int Delay;                     //for delay action in ms
int Start;
int Stop;
int Prio;                      //Priority of signal 0 - no priority
int For_Remove;                // mar for remove from tasks
int Executed;                       
uint Trigger;                  //signal has trigger state
//struct Signal *SA_ptr; //pointer to signals array
//struct Mb_Event *MbEv_ptr; //pointer to event array
} Discrete_Signals_t;
                        
Discrete_Signals_t args[MAX_Signals]; // Create array of Tasks to control signals by threads
                         
                         

//command intrepretator
char cmd_list[BUFFER];

 //установка локального времени
int set_cmd( char *str)
{
  char buff[512];
  char cmdline[400];	
  FILE *popen_result;
	printf ("start set_cmd \n");
	printf ("linux command get remote cmd: %s \n",str);
	memset(cmdline, 0, sizeof(cmdline));          // очистка буфера для вывода.
	strcat (cmdline,str);
	//printf ("created cmd string: %s \n",cmdline);
        popen_result = popen(cmdline, "r");
        if(!popen_result){
            printf("Can't execute command \n");
           return 0;
        }

	while(fgets(buff, strlen(buff), popen_result)!=NULL){
        printf("%s", buff);
        }
    pclose(popen_result);
    return 1;
}

//get system time and date
int get_cmd(char *str, char *retval ){
//time section
FILE *popen_result;
char tbuff[512]=" ";
	popen_result = popen(str, "r");
        if(!popen_result){
           printf("server Can't execute command \n");
  	   return 1;
	  }
	while(fgets(tbuff, strlen(tbuff), popen_result)!=NULL) {
  	      printf("%s", tbuff);
	      strcat(retval,tbuff);
  	    }
	pclose(popen_result);
return 0;			
}


//---------------------
 //установка локального времени
int set_date( char *str)
{
	printf ("start set_date \n");
	FILE *popen_result;
        char buff[512];
	char cmdline[400];
	//char *lincmd="date +'%Y-%m-%d %H:%M:%S' -s '";
	char *lincmd="date -s '";
	printf ("linux command get format: %s \n",lincmd);
	printf ("linux command get remote time: %s \n",str);
	//сhar * strcpy (char *to, const char *from) (функция)
	memset(cmdline, 0, sizeof(cmdline));          // очистка буфера для вывода.
	strcat (cmdline,lincmd);
	//printf ("created string %s \n",cmdline);
	strcat (cmdline,str);
	//printf ("created string %s \n",cmdline);
	//strcpy (cmdline,"'");
	strcat (cmdline,"'");
	printf ("created cmd string: %s \n",cmdline);
        popen_result = popen(cmdline, "r");
    
    if(!popen_result){
        printf("Can't execute command \n");
        return 1;
    }
	while(fgets(buff, strlen(buff), popen_result)!=NULL){
              printf("%s", buff);
        }
    pclose(popen_result);
    return 0;
	
}

//get system time and date
int get_date(char *retval){
	//time section
FILE *popen_result;
char tbuff[512]=" ";
	popen_result = popen("date +'%Y-%m-%d %H:%M:%S'", "r");
        if(!popen_result){
           printf("server Can't execute command \n");
   	   return 1;
	}
	while(fgets(tbuff, strlen(tbuff), popen_result)!=NULL) {
 	      printf("%s", tbuff);
 	      strcat(retval,tbuff);
	     }
	pclose(popen_result);
		
return 0;				
}

int load_cmdList ()
{
   char buf[]="date|date -r|pwd |ls -l|top |help ";
   char buf2[35],*ptr;
   int i;
    
   for(ptr = strtok(buf,"|"); ptr != NULL; ptr = strtok(NULL,"|"))
   {
    printf ("command: %s\n",ptr);
        strcpy(buf2,ptr);
	strcpy(cmd_list,ptr); //заполняем список - лист команд
        printf("word:");
        for(i=0;i<35;i++)
        {
        if(buf2[i] == ' ')
          break;
        else
          printf("%c",buf2[i]);
        }
        printf("\n------------------------\n");
		printf("cmd_list %s \n\r",cmd_list);
   }
    
return 0;	
}

int cmd_selector( char *str) {
	
		if (strcasestr(str,cmd_list)!=0) {
		printf ("Command found in cmd_list %s \n",str);
		return 1;
		}
	
	
	
return 0;

}

//the thread function
void *connection_handler(void *);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    
    //check input parameters 
    //    if (m_argc != 2) {
    //    fprintf(stderr,"usage: <server> <port> \n");
    //    exit(1);
    //} 
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
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    printf("[Waiting for incoming connections:%i] ...\n\r",PORT);
    //c = sizeof(struct sockaddr_in);

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
        if (client_sock < 0)
          {
            perror("accept failed");
            return 1;
           }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * 
 */

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , *msgtmp , client_message[BUFFER];
     
     //Create socket
    int sock_client
    sock_client = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock_client , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected with TCPCache Server\n\r");
    //send(sock , pHello , strlen(pHello) , 0) ;
    //keep communicating with server
    int sended_signals=0;
    
	
    
    while( (read_size = recv(sock , client_message , BUFFER , 0)) > 0 )
    {

		//coomand code event manager
		//if (cmd_selector(client_message)==0)  //if returned 0 ,it means not recognized command
		        printf("readed bytes %i \n\r",read_size);
		          
		          
		          
		        printf("\n\r------------------------ start packet %i bytes ----------------------------\n\r",read_size); 
                        int cn=0;
                        for (cn = 0; cn < read_size; cn++)
                          {
                            printf( " TCP_%i ",cn);
                            printf(" [%x] " ,client_message[cn]);                                               
                          }
                        printf("\n\r------------------------ end packet ----------------------------\n\r");
                        
			while(1) //send to cache server
                        {
                          strcat (message,"signal_read:wagoTCP.rl_oil_pump");
        
                               if( send(sock_client , message , strlen(message) , 0) < 0)
                                 {
                                   puts("Send failed");
                                    return 1;
                                  }
        
                              //Receive a reply from the server
                               if( recv(sock_client , server_reply , 10000 , 0) < 0)
                                  {
                                   puts("recv failed");
                                   break;
                                  }
								  
                        printf("Server reply: \n\r");
                        //puts("Server reply :");
                        puts(server_reply);
                                
	                        if( strstr(pSR,pMok) != NULL)
	                           {
	                              printf("Server reply error\n\r");
	                              break;
	                            }
	                  memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
                         }
						 


    }
     shutdown(sock_client, SHUT_RDWR); //close client socket
     close(sock_client);
     
    if(read_size == 0)
    {
        printf("Client disconnected read_size = 0 \n\r");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}
