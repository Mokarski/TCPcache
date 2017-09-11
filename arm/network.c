//NETWORK API
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include "signals.h"
#include "network.h"

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
                                                                                      
                                                                                      


int socket_init2()
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


int socket_init(char *ipaddr)
{

//    struct sockaddr_in server;
//    char duffer[10000];
//    char message[10000];
//	char server_reply[10000] = "";
//	char *pSR = server_reply;
//	char *pMok = "Ok!\n";
//	char *pHello;
//	pHello = "~core";
	
	
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr(ipaddr);
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


int Data_to_sName( char* tcp_buffer ){ //extract from tcp buffer signal and put to Name field
char sep[10]=";";
char *istr;

int signal_counter=0;

        // Выделение первой части строки
           istr = strtok (tcp_buffer,sep);
           if ( istr == NULL ) {
              printf ("DATA_to_sName: istr1 NULL \n\r");
              return 1;
              //sDeSerial_by_num (signal_counter);
              }
            strcpy (Signal_Array[signal_counter].Name,istr); //copy 1 signal string to Signal_Array field Name
            signal_counter++; //increment before while
            
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
        strcpy (msg, "signal_read;");
        strcat (msg,message_in);
        //printf("tcpsignal_read: Server Signal_Array [%s] \n\r",message); //debug
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
	if ( strstr (pSR,"Ok!") != NULL ){
	    printf("SERVER reply: Ok!\n\r");
	   }
	memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
return 0;
}


int frame_read_s (char *message_in, short int signals_count){
    char message [MAX_MESS]; //max size for 1 packet send for tcp is 65534 bytes 
    char c_len[4];
    char c_count[4];
    if ( strlen (message_in) >  MAX_MESS ) {
        printf("Exeeded maximum buffer size, message_size: %i > then buffer_size:%i",strlen (message_in), MAX_MESS);
        return -1;
       }
    strcpy (Frame_Container.type,"rd");
    Frame_Container.count = signals_count;
    strcpy (Frame_Container.data, message_in);
    Frame_Container.len = strlen (Frame_Container.data) + strlen (Frame_Container.type);
    printf("Frame[ type:%s len:%i count:%i data{ %s } ] \n\r",Frame_Container.type, Frame_Container.len, Frame_Container.count, Frame_Container.data);
    
    //Try Construct frame
    //Frame_Container.type, Frame_Container.len, Frame_Container.count, Frame_Container.data
    strcpy(message,Frame_Container.type);
    if ( Frame_Container.len < 10 ){
         //strcpy(c_len,"0"); //add a zerro 
         ItoA(Frame_Container.len,c_len); //convert int to char
         c_len[1]=c_len[0]; //copy converted number into high part of array
         c_len[0] = '0'; //adding a zerro at low side
         } else ItoA(Frame_Container.len,c_len); //convert int to char         
    strcat(message,c_len);
    
    if ( Frame_Container.count < 10 ){
        //strcpy(c_count,"0"); //add a zerro at first empty byte
        ItoA(Frame_Container.count,c_count); //convert int to char
        c_count[1]=c_count[0];
        c_count[0]='0';
       } else ItoA(Frame_Container.count,c_count); //convert int to char
    strcat(message,c_count);
    
    strcat (message, Frame_Container.data);
    printf("Constructed Frame^[%s] \n\r",message);
return 0;
}

int frame_tcpreq (char *msg){
 char *pSR = server_reply;
 int ret=0;
 
    if( send(sock , msg , strlen(msg) , 0) < 0)
        {
            puts("Send request to CacheServer failed!!!");
            return -1;
            //break;
         } else { printf ("[ Send to SRV ]: {%s} \n\r",msg); }
        
        //Receive a reply from the server
        if( recv(sock , server_reply , MAX_MESS , 0) < 0) // recive message from server and put into global array
        {
            puts("recv from CacheServer failed!!!");
            return -1;
            //break;
        } else { printf ("[ SRV reply ]: {%s} \n\r",server_reply); ret=1; }
        
        if ( strlen (server_reply) > 5){ //if response from server more then 4 symbols, then we get signals
            strcpy(signal_parser_buf, server_reply); //copy to global array 
            ret=2;
            }
    
	if( strstr(pSR,"Ok!") == NULL)
	{
	  printf("Server reply error: not Ok!\n\r");
	  return -2;
	  //break;
	}
	
	if ( strstr (pSR,"Ok!") != NULL ){
	    printf("SERVER reply: Ok!\n\r");
	    ret = 3;
	   }
	memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
return ret;
}


int frame_tcpsend (char *msg){
 char *pSR = server_reply;
 int ret=0;
 
    if( send(sock , msg , strlen(msg) , 0) < 0)
        {
            puts("Send request to CacheServer failed!!!");
            return -1;
            //break;
         } else { printf ("[ Send to SRV ]: {%s} \n\r",msg); }
        
        //Receive a reply from the server
        int cnt=0;
        while (cnt != 6)
        {
        
         if( recv(sock , server_reply , strlen(server_reply) , 0) < 0) // recive message from server and put into global array
           {
            puts("recv from CacheServer failed!!!");
            return -1;
            //break;
            } else { printf ("[ SRV reply ]: {%s} \n\r",server_reply); ret=1; }
        
          if ( strlen (server_reply) > 5){ //if response from server more then 4 symbols, then we get signals
              strcpy(signal_parser_buf, server_reply); //copy to global array 
              ret=2;
              }
    
	   if( strstr(pSR,"Ok!") == NULL)
	      {
	       printf("Server reply error: not Ok!\n\r");
	       return -2;
	       //break;
	      }
	
	    if ( strstr (pSR,"Ok!") != NULL ){
	        printf("SERVER reply: Ok!\n\r");
	        ret = 3;
	       }
	     cnt++;
	  }
	memset(server_reply, 0, sizeof(server_reply) / sizeof(server_reply[0]));
    //}
    
return ret;
}



int frame_pack (char *type, char *message_in, char *message_out) { //construct frame frome message_in and put result to message type is "rd" or "wr"
    //char message [MAX_MESS]; //max size for 1 packet send for tcp is 65534 bytes 
    char c_len[4];
    char c_count[4];
    
    if ( strlen (message_in) >  MAX_MESS ) {
        printf("Exeeded maximum buffer size, message_size: %i > then buffer_size:%i",strlen(message_in), MAX_MESS);
        return -1;
       }
    strcpy (Frame_Container.type,type);
    Frame_Container.count = 0;
    strcpy (Frame_Container.data, message_in);
    Frame_Container.len = strlen (Frame_Container.data) + 1; //length = Data container size in bytes + 1byte =";"
    //printf("Frame[ type:%s len:%i count:%i data{ %s } ] \n\r",Frame_Container.type, Frame_Container.len, Frame_Container.count, Frame_Container.data);
    
    //Try Construct frame
    //Frame_Container.type, Frame_Container.len, Frame_Container.data
    strcpy(message_out,Frame_Container.type);
    strcat (message_out,";");
    ItoA(Frame_Container.len,c_len); //convert int to char
    strcat(message_out,c_len);    
    strcat (message_out, "#");
    strcat (message_out, Frame_Container.data);
    strcat (message_out,";\0");
    printf("Constructed Frame^[%s]\n\n\r",message_out);
    return 0;
}

int frame_unpack (char *server_reply, char *data){ // copy serialized signals into data and return 1 if read or 2 if write
    char sep[10]="#";
    char type[5];
    char *istr;
    char header[100];
    char c_len[10];
    int  ret_rd_wr=0;
    istr = strtok (server_reply,sep); //extract HEADER and DATA by "#"
    if (istr != NULL) { //HEADER
        printf("Header^{%s}\n\r",istr);
        strcpy(header,istr);
        } else {  printf ("data_extract: Header - NULL! \n\r");
                  return -1;
               }
    
    istr = strtok (NULL,sep);
    if (istr != NULL) { //DATA
        printf("PACKET^{%s}\n\r",istr);
        strcpy (data,istr);
        } else { printf ("data_extract: PACKET - NULL! \n\r");
                 return -1;
               }
    
    
    
    char sep2[10]=";";           
    istr = strtok (header,sep2); //extract type and number bytes
    if (istr != NULL) { //type of frame read or write
        printf("Type^{%s}\n\r",istr);
        strcpy(type,istr);
        if (strstr(type,"rd")) ret_rd_wr=1; //read request
        if (strstr(type,"wr")) ret_rd_wr=2; //write request
        if (strstr(type,"ok")) ret_rd_wr=3; //ack - ok
        if (strstr(type,"err")) ret_rd_wr=4; //server return "error request"
        if (strstr(type,"ret")) ret_rd_wr=5; //retry request, if packet len no OK
        } else {  printf ("data_extract: Header1 - NULL! \n\r");
                  return -1;
               }
               
               
    istr = strtok (NULL,sep2);    
        if (istr != NULL) { //extrcat lenght 
        printf("Len^{%s}\n\r",istr);
        strcpy(c_len,istr);
        printf("c_len {%s}\n\r",c_len);
        } else {  printf ("data_extract: Header2 - NULL! \n\r");
                  return -1;
               }
               
         int r1= atoi (c_len); //number bytes in packet;
         int r2= strlen (data);
         //printf ("[r1-%i] [r2-%i]\n\r",r1,r2);
         
         if ( r1 != r2 ){
             printf("ERR Recived bytes != calculated | [%i != %i] \n\r",r1,r2);
             return -2; //if data length != calculated data lenght
             }
     
     
    return ret_rd_wr;
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
        strcpy (message,"signal_write;");    
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
        strcpy (message2,"signal_write;");    
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
	
	if( strstr(server_reply,"Ok!") != NULL)		
	  {
	    printf("SERVER reply: Ok!\n\r");
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
