//NETWORK API
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include "signals.h"
#include "network.h"

//char  message[2000];
//char signal_buffer[20000];
//char signal_parser_buf[90000] = "";
//int sock;
//struct sockaddr_in server;

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
                                                                                      
                                                                                      

int socket_init()
{

//    struct sockaddr_in server;
//    char duffer[10000];
//    char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
	char *pHello;
	pHello = "~core";
	
	
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected with TCPCache Server\n\r");
    //send(sock , pHello , strlen(pHello) , 0) ;
    //keep communicating with server

return 0;
}

int socket_close(){
shutdown(sock, SHUT_RDWR);
close(sock);
puts("Socket closed");
return 0;
}

int tcp_rw(char *message_in){
        char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
	char *pHello;
	pHello = "~core";
        int sended_signals=0;
        int signal_counter=0;

    while(1)
    {
        strcat (message,"signal_read:wagoTCP.rl_oil_pump");
        
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        
        //Receive a reply from the server
        if( recv(sock , server_reply , 10000 , 0) < 0)
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
	signal_counter++;
    }
return 0;
}

struct prototype {
char Name[500];
char chVal[100];
int  iVal[2];
} proto_arr[MAX_Signals];

int signal_devider (){
char sep2[10]=" ";
char sep3[10]=":";
char line[500];
char *istr2;
char *istr3;
int signal_counter;
for ( signal_counter = 0; signal_counter < MAX_Signals; signal_counter++ ){
           //explode signal to name field and val field
           strcpy(line,proto_arr[signal_counter].Name);
           istr2 = strtok (line,sep2);
           strcpy (proto_arr[signal_counter].Name,istr2);
           istr2 = strtok (NULL,sep2);
           strcpy (proto_arr[signal_counter].chVal,istr2);

           //extract digits from struct "value:num" line
           istr3 = strtok (proto_arr[signal_counter].chVal,sep3);
           istr3 = strtok (NULL,sep3);
           proto_arr[signal_counter].iVal[1] = atoi (istr3);
           
           printf("DEVIDER_[#%i] NAME: [%s] chVal: [%s] iVal[%i] \n\r",signal_counter ,proto_arr[signal_counter].Name, proto_arr[signal_counter].chVal, proto_arr[signal_counter].iVal[1] );                          
           }
return 0;
}


int tcpsignal_parser( char* tcp_buffer ){ //extract from tcp buffer signal and put to Name field
char sep[10]=";";
char *istr;

int signal_counter=0;

        // Выделение первой части строки
           istr = strtok (tcp_buffer,sep);
           if ( istr == NULL ) {
              printf ("tcpsignal_parser: istr1 NULL \n\r");
              return 1;
              //sDeSerial_by_num (signal_counter);
              }
            strcpy (Signal_Array[signal_counter].Name,istr); //copy 1 signal string to Signal_Array field Name
             
        // Выделение последующих частей
                 while ( istr != NULL )
                    {
                       istr = strtok (NULL,sep);   
                       
                          if ( istr == NULL ) 
                             {
                               printf ("[#%i] End list \n\r",signal_counter);
                               return 1;
                             }
                             
                        strcpy (Signal_Array[signal_counter].Name,istr); //copy 1 signal string to Signal_Array field Name
                          
                        
                          
                       signal_counter++;
                     }
              
        

return 0;
}

int tcpsignal_read(char *message_in){
        char msg[10000];
	char server_reply[20000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!";
	char *pHello;
	char *istr;
	char *istr2;
	char sep[10] =";";
	char sep2[10] =" ";
	pHello = "~core";
        int sended_signals=0;
        int signal_counter=0;

    //while(1)
    //{
        strcpy (msg,"");
        strcpy (msg, "signal_read:");
        strcat (msg,message_in);
        printf("-Read from Server Signal_Array [%s] \n\r",message);
        if( send(sock , msg , strlen(msg) , 0) < 0)
        {
            puts("Send request to CacheServer failed");
            return 1;
            //break;
        }
        
        //Receive a reply from the server
        if( recv(sock , server_reply , 20000 , 0) < 0)
        {
            puts("recv from CacheServer failed");
            return 1;
            //break;
        }
        //printf("Server reply: \n\r");
        //puts("Server reply :");
                    //        puts(server_reply);
        
        
        strcpy(signal_parser_buf, server_reply);
        //puts(server_reply);
         // printf ("signal_parser_buf:[%s] \n",signal_parser_buf);
        
	if( strstr(pSR,pMok) == NULL)
	{
	  printf("Server CMD_READ reply error\n\r");
	  return 2;
	  //break;
	}
	memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
return 0;
}

int tcpsignal_write(char *message_in, int iVal){
        speedtest_start();
        char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!";
	char *pHello;
	char cVal[5];
	pHello = "~core";
        int sended_signals=0;

    //while(1)
    //{
        strcpy (message,"signal_write:");    
        strcat (message,message_in);
        ItoA(iVal,cVal);
        strcat (message,":");    
        strcat (message,cVal);
//        puts("Send write ");
        //printf("MESSAGE WRITE [%s]\n\r",message); //DEBUG
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send write request failed");
            return 1;
            //break;
        }
        
        //Receive a reply from the server
        if( recv(sock , server_reply , 10000 , 0) < 0)
        {
            puts("recv from CacheServer failed");
            printf("Server reply: [ %s ] \n\r",server_reply);
            return 1;
            //break;
        }
         //printf("Server reply: [ %s ] \n\r",server_reply);
        
//       puts("Server reply :");
//	if( strstr(pSR,pMok) != NULL)

	if( strstr(server_reply,"Ok!") == NULL)	
	{
	  printf("Server reply error? - no OK! \n\r");
	  printf("-> reply: [ %s ] \n\r",server_reply);
	  return 1;
	  //break;
	}
	//memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
     printf(" ++++++++++++++++++++++++==>   SPEEDTEST Function Time: [ %i ] ms. \n\r", speedtest_stop());
     
return 0;
}

int tcpsignal_packet_write(char *message_in ){
        speedtest_start();
        char message2[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!";
	char *pHello;
	char cVal[5];
	pHello = "~core";
        int sended_signals=0;

    //while(1)
    //{
        strcpy (message2,"signal_write:");    
        strcat (message2,message_in);
//        ItoA(iVal,cVal);
//        strcat (message2,":");    
//        strcat (message,cVal);
//        puts("Send write ");
        printf("MESSAGE WRITE [%s]\n\r",message2); //DEBUG
        if( send(sock , message2 , strlen(message2) , 0) < 0)
        {
            puts("Send write request failed");
            return 1;
            //break;
        }
        
        //Receive a reply from the server
        if( recv(sock , server_reply , 10000 , 0) < 0)
        {
            puts("recv from CacheServer failed");
            printf("Server reply: [ %s ] \n\r",server_reply);
            return 1;
            //break;
        }
         //printf("Server reply: [ %s ] \n\r",server_reply);
        
//       puts("Server reply :");
//	if( strstr(pSR,pMok) != NULL)

	if( strstr(server_reply,"Ok!") == NULL)	
	{
	  printf("Server reply error? - no OK! \n\r");
	  printf("->Server reply: [ %s ] \n\r",server_reply);
	  return 1;
	  //break;
	}
	/*
	if( strstr(server_reply,"Err!") == NULL)	
	{
	  printf("Server reply Err! \n\r");
	  printf("->Server reply: [ %s ] \n\r",server_reply);
	  return 1;
	}
	*/
	//memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
     printf(" ++++++++++++++++++++++++==>   SPEEDTEST Function Time: [ %i ] ms. \n\r", speedtest_stop());
     
return 0;
}
