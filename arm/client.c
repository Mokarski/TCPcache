/*
 *   C ECHO client example using sockets
 */
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
 
#include "signals.h" //inet_addr

char message[2000];

int build_packet(char input_str[2000]){
char buffer[2000]="";
 printf("Start packet assembly...\n\r");
        
        strcpy(buffer,  "start_set_signals:");
       //printf ("Packet: [%s] \n\r",duffer);
        strcat (buffer,input_str);
	strcat (buffer,":end_set_signals");
        printf ("Packet: [%s] \n\r",buffer);
//        memcpy (message,buffer,2000);    
        strcat (message,buffer);            
	printf ("SENDPacket: [%s] \n\r",message);
 return 0;
}

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char duffer[10000];
    char message[10000];
//    char *message_start;
//    char *message_end;
	char server_reply[10000] = "";
	char *pSR = server_reply;
	char *pMok = "Ok!\n";
//	message = "";
//        message_start = "start_set_signals:"; 
//        message_end = ":end_set_signals";
	char *pHello;
	pHello = "~core";
	
	
	
//	init_signals_list();
//	signals_file_load();
//	printf("Signals:\n\n\r");
//	print_signals_list(); //fill signals and echo to screen
	
	

	    
	
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
    int sended_signals=0;
    while(1)
    {
        //printf("Enter message: ");
        //scanf("%s" , message);
        //Send some data
        /*
        if (sended_signals < MAX_Signals){        
           char tmp[256];
            strcpy(tmp,Signal_Array[sended_signals].Name);
            printf(" Signal Name: [%s] Number [ %i ]\n\r",Signal_Array[sended_signals].Name, sended_signals);
            strcat(tmp,",0; ");
            //Signal_Array[n].Value[1]=sVal;
            //build_packet("rl.relay1,1;rl.relay2,2;rl.relay3,3;rl.relay4,4");            
            strcpy(message," "); //errase buffer
            //build_packet(tmp);
            strcat (message,"start_set_signals:");
            strcat (message,Signal_Array[sended_signals].Name);
            strcat (message,",0");
            strcat (message,":end_set_signals");
            printf("MESSAGE to send: \n\r %s \n\r",message);
            sended_signals++;
           }
        */
        
        
        //strcat (message,"signal_read:.");
        //strcat (message,"signal_write:.:2");
//        strcat (message,"signal_read:wagoTCP.rl_oil_pump");
        //strcat (message,"signal_write:wagoTCP.rl_oil_pump:9999");
                strcat (message,"signal_read:rpdu485с.kei.joy_");
        
        //strcat (message,"signal_read:wagoTCP.");
        //wagoTCP.rl_oil_pump
        
        
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
	shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}