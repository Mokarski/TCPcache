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

char signal_parser_buf[90000];
//struct sockaddr_in server;
char duffer[10000];
char message[10000];
char server_reply[10000];
//char *pSR = server_reply;
//char *pMok = "Ok!\n";
//	char *pHello;
//	pHello = "~core";


//char message[2000];
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
int tcpsignal_parser( char* tcp_buffer );
int tcpsignal_read(char *message_in);
int tcpsignal_write(char *message_in, int iVal);

#endif