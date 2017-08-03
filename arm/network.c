/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>

#    struct sockaddr_in server;
        char duffer[10000];
        char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
	char *pHello;
	pHello = "~core";

#char message[2000];
int sock;
struct sockaddr_in server;

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
        char duffer[10000];
        char message[10000];
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
    }
}


int tcpsignal_read(char *message_in){
        char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
	char *pHello;
	pHello = "~core";
        int sended_signals=0;

    while(1)
    {
        strcpy (message, "signal_read:");
        strcat (message,message_in);
        
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
    }
}

int tcpsignal_write(char *message_in, int iVal){
        char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
	char *pHello;
	char cVal[5];
	pHello = "~core";
        int sended_signals=0;

    while(1)
    {
        strcpy (message,"signal_write:");    
        strcat (message,message_in);
        ItoA(iVal,cVal);
        strcat (message,":");    
        strcat (message,cVal);
        printf("MESSAGE WRITE [%s]\n\r",message);
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
    }
}

