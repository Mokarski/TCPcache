/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
//#include "/home/opc/Kombain/test/include/modbus/modbus.h"
//#include "network.h"
#include "signals.h"

char  message[2000];
//char signal_buffer[20000];
char signal_parser[90000] = "";
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

int tcpsignal_parser( char* tcp_buffer ){
char sep[10]=";";
char *istr;

int signal_counter=0;


        // Выделение первой части строки
           istr = strtok (tcp_buffer,sep);

//        if (strlen (istr) > 1) {
             printf("===== [%i] istr [%s] \n\r",signal_counter,istr);
             strcpy (proto_arr[signal_counter].Name, istr);
             //signal_devider (proto_arr[signal_counter].Name, signal_counter);
             
//           }
      // Выделение последующих частей
                 while ( istr != NULL )
                    {
                       // Вывод очередной выделенной части
                       //printf ("[%i] tcp_parser read [%s] \n",signal_counter,istr);
                       
                             //to do: fill the signals struct
                             // Выделение очередной части строки
                             
                       
                       istr = strtok (NULL,sep);   
                       if ( istr == NULL ) 
                          {
                            printf ("end list \n\r");
                            break;
                            }
                       if ( strlen ( istr ) > 1 ){                    
                          strcpy (proto_arr[signal_counter].Name,istr);
                          printf("[#%i] NAME: [%s] chVal: [%s] iVal[%i] \n\r",signal_counter ,proto_arr[signal_counter].Name, proto_arr[signal_counter].chVal, proto_arr[signal_counter].iVal[1] );                          
                          
                          } 
                          
                       signal_counter++;
                     }
              
        

return 0;
}

int tcpsignal_read(char *message_in){
        char message[10000];
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
        strcpy (message, "signal_read:");
        strcat (message,message_in);
        
        if( send(sock , message , strlen(message) , 0) < 0)
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
        
        
        strcpy(signal_parser, server_reply);
        //puts(server_reply);
          printf ("signal_parser:[%s] \n",signal_parser);
        
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
        printf("MESSAGE WRITE [%s]\n\r",message);
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
            return 1;
            //break;
        }
         printf("Server reply: [ %s ] \n\r",server_reply);
        
//       puts("Server reply :");
//	if( strstr(pSR,pMok) != NULL)

	if( strstr(server_reply,"Ok!") == NULL)	
	{
	  printf("Server reply error? no OK! \n\r");
	  return 1;
	  //break;
	}
	memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
return 0;
}

int main(int argc , char *argv[])
{
//    int sock;
    struct sockaddr_in server;
    char duffer[10000];
    char message[10000];
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
	char *pHello;
	pHello = "~core";
     
     
    printf("MAX_Signals [%i] \n",MAX_Signals);
	    
//    print_sginals_list();
    socket_init();
    tcpsignal_write("485",3);
    socket_close();

    socket_init();
//    tcpsignal_read("485");
    
    
    if ( tcpsignal_read("485") == 0 ){ // if we get response from server
        tcpsignal_parser(signal_parser);
    }
    socket_close();
    
        signal_devider();

    /*
    shutdown(sock, SHUT_RDWR);
    close(sock);
    */
//    print_sginals_list();
//printf("1 %s \n", Signal_Array[0].Name);
//printf("2 %s \n", Signal_Array[1].Name);
//printf("3 %s \n", Signal_Array[2].Name);
//printf("4 %s \n", Signal_Array[3].Name);

return 0;
}
