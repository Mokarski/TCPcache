//https://github.com/spetca/socketsChat/blob/master/sthread.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#define BFSZ 100
void *connection_handler(void *);

int main(int argc, char *argv[])
{
	int socket_desc,new_socket,c,*new_sock;
	struct sockaddr_in server, client;
	char *message;
	char buffer[BFSZ];
	int client_count  =0;
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc < 0)
	{
		puts("could not create socket");
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	if(bind(socket_desc,(struct sockaddr *)&server, sizeof(server))<0)
	{
		puts("bind fail");
		return 1;
	}
	puts("bind complete");
	
	listen(socket_desc,3);
	
	puts("waiting for connections...");
	c=sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Connection accepted");
		snprintf(buffer, BFSZ, "hello client: %d\n", client_count++);
		//message = "hello client received msg\n";
		write(new_socket, buffer,strlen(buffer));

		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;
		
		if( pthread_create( &sniffer_thread, NULL, connection_handler, (void*)new_sock))
		{
			puts("could not create thread");
			return 1;
		}

		puts("handler assigned");
	}

	if(new_socket <0){
		puts("accept failed");
		return 1;
	}
	close(socket_desc);
	close(new_socket);
	close(c);
	return 0;
}

void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	printf ("Socket ID[#%i]\n\r",sock);
	char *message, client_message[2000], server_message[2000];
	int read_size;
	message = "i'm ur connection handler! lol\n";
	write(sock, message, strlen(message));

	message = "type something to send...\n";
	write(sock, message, strlen(message));

	while( (read_size = recv(sock, client_message,2000,0)) >0)
	{
		memset(&server_message[0], 0, sizeof(server_message));
		printf("received:%s\n",client_message);
		memset(&client_message[0], 0, sizeof(client_message));
		printf("send:");
		fgets(server_message,sizeof(server_message), stdin);
	
		write(sock,server_message, strlen(server_message));
	}

	if(read_size == 0)
	{
		puts("client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		puts("recv error");
	}

	free(socket_desc);
	return 0;
}
