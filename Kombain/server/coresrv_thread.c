/*
    C socket server example, handles multiple clients using threads
*/
 // WARNING - 
 // gcc -pthread -o term term.c
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
 
#define PORT 3490  // порт, на который будут приходить соединения
#define BUFFER 2000  // input buffer size in bytes
//command intrepretator
 //установка локального времени
 
 // Signal specification
 
typedef  struct Signal {
	 char *Ch_sname; //signal name
	 int I_prio;
	 int I_critical;
	 int I_task_id;          // номер задачи
	 int I_destination_id [3];   // кому принадлежит, маршрутизируем адресату
	 int I_creation_id;      // кто создал сигнал
	 int I_var;              // int значение сигнала
	 char *Ch_var;        // char значение сигнала
	 int I_alarm;            // Пред аварийное сосотояние
	 int I_warning;          // Авария
	 int I_low_range;        // нижняя граница сигнала
	 int I_high_range;       // верхняя граница сигнала
	 char *Ch_exec_id;        // подпись последнего обработчика
 }Signal;
 
int set_cmd( char *str)
{
	printf ("start set_cmd \n");
	FILE *popen_result;
    char buff[512];
	char cmdline[400];	
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
    //while(fgets(buff, sizeof(buff), popen_result)!=NULL){
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
	char *lincmd="date +'%Y-%m-%d %H:%M:%S' -s '";
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
    //while(fgets(buff, sizeof(buff), popen_result)!=NULL){
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


int cmd_selector( char *str) {
	
	 //strcmp ("привет", "привет")
     // => 0 /* Эти две строки одинаковы. */
	//commands^
	//get - get date local
	//set - set local date
	//printf ("input command recived %s \n",str);
	if (strcasestr(str,"[get_date]")!=0) {
		//printf ("getdate command recived %s \n",str);
		return 2;
	}
	
	if (strcasestr(str,"[set_date]")!=0) {
		//printf ("setdate command recived %s \n",str);
		return 1;		
	}
	
	if (strcasestr(str,"[cmd_in]")!=0) {
		//printf ("cmdin command recived %s \n",str);
		return 3;		
	}
	
	if (strcasestr(str,"[cmd_out]")!=0) {
		//printf ("cmdout command recived %s \n",str);
		return 4;		
	}
	
	return 0;
	
}

//the thread function
void *connection_handler(void *);

//print signal
void print_signal ( Signal model ) {
  printf(
  "<signal name=\"%s\">\n"
  "    <make prio=\"%i\">\n"
  "        <critical>%i</critical>\n"
  "        <task_id>%i</task_id>\n"  
  "        <char val>%s</char val>\n"
  "    </make>\n"
  "    <creation_id>%i</creation_id>\n"
  "    <exec_id>%s</exec_id>>\n"
  "    <destination_parent_id>%i</destination_parent_id>>\n"
  "    <destination_child_id>%i</destination_child_id>>\n"
  "</model>", model.Ch_sname, model.I_prio, model.I_critical, model.I_task_id, model.Ch_var, model.I_creation_id, model.Ch_exec_id,model.I_destination_id[0],model.I_destination_id[1]);
}
 
void destination_route(Signal* model, int parent_id, int child_id){
         model.I_destination_id[0] = parent_id;
         model.I_destination_id[1] = child_id;
}
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    //----------test signal structure
    struct Signal Array_Signal[300]; //number signals
	Array_Signal[0].Ch_sname="Test signal";
	Array_Signal[0].I_prio=1;
	Array_Signal[0].I_destination_id[0]=11;
	Array_Signal[0].I_destination_id[1]=22;	
	destination_route(Array_Signal[0],33,22);
	print_signal(Array_Signal[0]);
	
//	printf("Signal Name: %s \n",Array_Signal[0].Ch_sname);
//	printf("Signal I_prio: %i \n",Array_Signal[0].I_prio);
	//------------------------------------------------
	//check input parameters 
	//if (m_argc != 2) {
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
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
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
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , *msgtmp , client_message[BUFFER];
     
    //Send some messages to the client
    message = "connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type command: \n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , BUFFER , 0)) > 0 )
    {
		//coomand code event manager
		if (cmd_selector(client_message)==0) { //if returned 0 ,it means not recognized command
			printf( "command not executed %s \n",client_message);
		} 
		
		if (cmd_selector(client_message)==1) {
			printf( "set - %s \n",client_message);
			strcat(client_message, "set_date - %s \n");
		}
		if (cmd_selector(client_message)==2) {
			printf( "get - %s \n",client_message);
			get_date(msgtmp);
			printf( "in msgtmp: - %s \n",msgtmp);
			if (strlen(msgtmp)>4) {
				strcat(client_message, msgtmp);
			}	
		} 
		
		if (cmd_selector(client_message)==3) {
			printf( "get_cmd - %s \n",client_message);
			strcat(client_message, "get_cmd - %s \n");
		} 
		
		if (cmd_selector(client_message)==4) {
			printf( "set_cmd - %s \n",client_message);
			strcat(client_message, "set_cmd - %s \n");
		} 
		
        //Send the message back to client
		printf( "send to client: - %s \n", client_message);
        write(sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
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
