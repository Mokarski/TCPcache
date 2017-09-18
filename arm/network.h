/*
 *   C ECHO client example using sockets
 */
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include "signals.h"

#ifndef _NETWORK_H
#define _NETWORK_H
#define MAX_MESS 30000 // maximum size of buffer for send or recive

#ifndef DEBUG 
#define DEBUG 0
#endif
char signal_parser_buf[MAX_MESS]; //global array
char message[MAX_MESS];           //global array
char server_reply[MAX_MESS];      //global array

struct frame {
char type[4]; // type rd or wr
short int  len; // number bytes - size 2 byte = 65535
short int  count; //number of signals - size 2 byte = 65535
char data[MAX_MESS]; //data container
};
struct frame Frame_Container;

int sock;
struct sockaddr_in server;

/*
 * приведение целого к строковому формату
  */
  
 void Reverse(char s[]);

 void ItoA(int n, char s[]);                                                          
                                                                                      
//  NETWORK
int socket_init();
int socket_close();
int frame_unpack (char *server_reply, char *data);

int tcpsignal_parser( char* tcp_buffer );
int tcpsignal_read(char *message_in);
int tcpsignal_write(char *message_in, int iVal);
int tcpsignal_packet_write(char *message_in);
#endif