//NETWORK API
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include "signals.h"
#include "network.h"


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

int result=0; //return number of founded signals
        // Выделение первой части строки
           istr = strtok (tcp_buffer,sep);
           if ( istr == NULL ) {
              printf ("!!!ERR extract signals, to name! DATA_to_sName: istr1 = NULL \n\r");
              return 1;
              //sDeSerial_by_num (signal_counter);
              }
            strcpy (Signal_Array[signal_counter].Name,istr); //copy 1 signal string to Signal_Array field Name
            signal_counter++; //increment before while
            result++;
        // Выделение последующих частей
                 while ( istr != NULL )
                    {
                       istr = strtok (NULL,sep);   
                       
                          if ( istr == NULL ) 
                             {
                               printf ("[#%i] End list \n\r",signal_counter);
                               return signal_counter;
                             }
                             
                        strcpy (Signal_Array[signal_counter].Name,istr); //copy 1 signal string to Signal_Array field Name                        
                        signal_counter++;
                        result++; //increment for return results
                     }
return signal_counter;
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
    if (DEBUG == 1) printf("Constructed Frame^[%s] \n\r",message);
return 0;
}

int frame_tcpreq (char *msg){
 char *pSR = server_reply;
 int ret=0;
 
    if( send(sock , msg , strlen(msg)+1, 0) < 0)
        {
            puts("Send request to CacheServer failed!!!");
            return -1;
            //break;
         } else { if (DEBUG == 1) printf ("[ Send to SRV ]: {%s} \n\r",msg); } //debug info
        
        //Receive a reply from the server
        if( recv(sock , server_reply , MAX_MESS , 0) < 0) // recive message from server and put into global array
        {
            puts("recv from CacheServer failed!!!");
            return -1;
            //break;
        } else { 
                  //printf ("[ SRV reply ]: {%s} \n\r",server_reply); 
                  ret=1; 
                  }
        
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
/*
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
    if (DEBUG == 1) printf("Constructed Frame^[%s]\n\n\r",message_out);
    return 0;
}
*/
/*
int strcpyN(int start_pos, char *str, char *result){
int n=0;
int inc=0;
strcpy(result,"");
printf("strcpyN: Size recived %i \n\r",strlen(str));
if (strlen(str) < 1) return -1;
for (n=0; n < (strlen(str)); n++)
    {
      if ( n >= start_pos ){
          result[inc]=str[n];
          inc++;
          //printf("strcpyN [%c] \n\r",result[inc]);
         }    
    }
    printf("strcpyN [%s] \n\r",result);
return 0;
}


int Nstcpy(int end_pos, char *str2, char *result2){
int n=0;
int inc=0;
    //strcpy(result2,"");
    if (strlen(str2) < 1) return -1;
    printf("Nstrcpy_in str[%s] pos[%i]\n\r",str2,end_pos);
    printf("Nstrcpy [%s] \n\r",result2);
    for (n=0; n != (strlen(str2)); n++)
        {
         // printf("%i [%c] \n\r",n,str[n]);
         if ( inc != end_pos ){
             result2[inc]=str2[n];
             printf("%i [%c] \n\r",n,result2[inc]);         
             printf("Nstrcpy [%s] \n\r",result2);
             inc++;
             } 
        }
    printf("Nstrcpy [%s] \n\r",result2);
return 0;
}
*/

int exploderL(char delimiter, char *inn, char *outt){
int n=0;
int inc=0;
char tmp_out[MAX_MESS]  = {0};

//    printf("EXPLODER_L d %c in[%s] out[%s] \n\r",delimiter,inn,outt); //debug
    if(strlen(inn) < 1) return -1;
    int len = strlen(inn);
    for (n=0; n < len; n++)
        {
//         printf( "EXPLODER_L #%i [%c] \n\r",n,inn[n]);
         //if(inn[n]==delimiter){
         if ( inn[n]==delimiter ) {
             strncpy(outt,tmp_out,(size_t)n);
             break;
             } else {
//                      printf("EXPLODER_L inner for[%i]  char[%c] inc[%i] \n\r",n,inn[n],inc); //debug
//                      printf("EXPLODER_L #%i out{%s} \n\r",n,tmp_out);  //debug
                      tmp_out[inc]=inn[n];
                      inc++;
//                      printf("EXPLODER_L outter %i [%c] out{%s} \n\r",n,inn[n],tmp_out);  //debug
                    } 
//             printf("EXPLODER_L out [%s] \n\r",tmp_out);

        }
return 0;
}

int exploderR(char delimiter, char *in, char *out){
int n=0;
int inc=0;
int start=0;
//    printf("EXPLODER_R d %c in[%s] out[%s] \n\r",delimiter,in,out);    
    if (strlen(in) < 1) return -1;
    for (n=0; n < (strlen(in)); n++)
        {
         // printf("%i [%c] \n\r",n,str[n]);
         
         if (start == 1 )
            {
             //strncpy(out,tmp_out,(size_t)n);            
//             substr(char * dst, const char * src, size_t pos, size_t cnt)
             substr(out,in,n,strlen(in)-1);
             //out[inc]=in[n];
             //inc++;
             break;
             }          
             
         if ( in[n]==delimiter ){ 
             start=1;         
             //strncpy(out,tmp_out,(size_t)n);
            }
        }
//       out[inc] = 0;        
//     printf("EXPLODER_R out [%s] \n\r",out);        

return 0;
}

int frame_pack (char *type, char *message_in, char *message_out) { //construct frame frome message_in and put result to message type is "rd" or "wr"
    //char message [MAX_MESS]; //max size for 1 packet send for tcp is 65534 bytes 
    char c_len[4]={0};
    char c_count[4]={0};

    if ( strlen (message_in) >  MAX_MESS ) {
        printf("Exeeded maximum buffer size, message_size: %i > then buffer_size:%i",strlen(message_in), MAX_MESS);
        return -1;
       }
    int len = strlen (message_in) + 1; //length = Data container size in bytes + 1byte =";"    
    //Try Construct frame
    //strcpy (message_out,"");  //$
    memset(message_out, 0, sizeof(message_out));
    strcat(message_out,type);
    strcat (message_out,";");
    ItoA(len,c_len); //convert int to char
    strcat(message_out,c_len);    
    strcat (message_out, "#");
    strcat (message_out,message_in);
    strcat (message_out,";\0"); //*
//    strcat (message_out,";\0");
    if (DEBUG == 1) printf("Constructed Frame^[%s]\n\n\r",message_out);
    if ( strlen(message_out ) < 30) printf("Constructed Frame^[%s]\n\n\r",message_out);
    return 0;
}

int frame_unpack (char *srvr_reply, char *dat){ // copy serialized signals into data and return 1 if read or 2 if write
    char sep ='#';
    char type[4]={0}; // rd / wr err/ ret /Ok! +1 end string = 4 char
    char istr[MAX_MESS]={0};
    char header[100]={0};
    char c_len[10]={0};
    int  ret_rd_wr=0;
    

    exploderL (sep,srvr_reply,header); //extract HEADER and DATA by "#" seporator
    if (header != NULL) { //HEADER
        printf("Header^{%s}\n\r",header);
         if ( strlen(header)<100 ){          
             if ( strlen(header) < 30 ) printf("(len<30) Header^{%s}\n\r",header);
             }else {printf ("ERR HEADER TOO BIG[%i] \n\r",strlen(header));}
           
       } else {  printf ("ERR data_extract: Header - NULL! \n\r");
                 printf (">>>>>>> ERR data_extract: \n\r->SRV REPLY:{%s}\n\r ->DATA:{%s}! \n\r",srvr_reply,dat);
                  return -1;
               }
    
    exploderR (sep,srvr_reply,istr);
    if (istr != NULL) { //DATA
        //printf("PACKET^{%s}\n\r",istr); //debug
        strcpy (dat,istr);
        if (strlen (dat) < 30) printf ("data_: {%s} \n\r",dat);
        } else { 
                 printf ("ERR data_extract: PACKET FROM FRAME  - NULL! \n\r");
                 printf (">>>>>>> ERR data_extract: \n\r->SRV REPLY:{%s}\n\r ->DATA:{%s}! \n\r",srvr_reply,dat);
                 return -1;
               }
    
    
    
    char sep2=';';           
    exploderL(sep2,header,type); //extract type CMD  by ";"

    if (type != NULL) { //type of frame read or write
        printf("Type^{%s}\n\r",type);
        if (strstr(type,"rd")) ret_rd_wr=1;  //read request
        if (strstr(type,"wr")) ret_rd_wr=2;  //write request
        if (strstr(type,"Ok!")) ret_rd_wr=3; //ack - ok
        if (strstr(type,"err")) ret_rd_wr=4; //server return "error request"
        if (strstr(type,"ret")) ret_rd_wr=5; //retry request, if packet len no OK
        } else {  printf ("ERR data_extract: Header_cmd - NULL! \n\r");
    		  printf (">>>>>>> ERR data_extract: \n\r->SRV REPLY:{%s}\n\r ->DATA:{%s}! \n\r",srvr_reply,dat);
                  return -1;
               }
               
               
     exploderR (sep2,header,c_len);    //extract number bytes by ";"
     if (strlen (c_len) > 9 )
        {
         printf("Too big number in packet len! More then 9 bytes! \n\r");
         return -1;
        }
        if (c_len != NULL) { //extrcat lenght 
        printf("Len^{%s}\n\r",c_len);
        } else {  printf ("ERR data_extract: Header_pkt_len - NULL! \n\r");
                  printf (">>>>>>> ERR data_extract: \n\r->SRV REPLY:{%s}\n\r ->DATA:{%s}! \n\r",srvr_reply,dat);
                  return -1;
               }
               
         int r1= atoi (c_len); //number bytes in packet;
         int r2= strlen (dat); //bad solution packet len - 1  "*"
         if ( r1 != r2 ){
             printf("ERR Recived bytes len[%i] != calculated bytes len[%i]  \n\r",r1,r2);
             printf("DATA:{%s}\n\n",dat);
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
