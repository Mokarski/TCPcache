#include <stdio.h>
#include <string.h>
#include <fcntl.h>  /* File Control Definitions          */
#include <termios.h>/* POSIX Terminal Control Definitions*/
#include <unistd.h> /* UNIX Standard Definitions         */
#include <errno.h>  /* ERROR Number Definitions          */

void com_setup50(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B50 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup75(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B75 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup110(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B110 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup134(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B134 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup150(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B150 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup200(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B200 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}
void com_setup300(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B300 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup600(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B600 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup1200(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B1200 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup1800(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B1800 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup2400(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B2400 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup4800(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B4800 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup9600(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B9600 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}


void com_setup19200(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B19200 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup38400(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B38400 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}

void com_setup57600(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B57600 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}



void com_setup115200(int fd,int WaitTime){

                  struct termios SerialPortSettings;
                  tcgetattr (fd, &SerialPortSettings);
                  SerialPortSettings.c_cflag = B115200 | CS8  | CLOCAL | CREAD;
                  SerialPortSettings.c_iflag = IGNPAR;
                  SerialPortSettings.c_oflag = 0;
                  SerialPortSettings.c_lflag = 0;
                  SerialPortSettings.c_cc[VMIN] = 0; //read 32 characters
                  SerialPortSettings.c_cc[VTIME] = WaitTime; //wait time
                  tcflush(fd,TCIFLUSH);
                  tcsetattr(fd, TCSANOW, &SerialPortSettings);

}


int test_transfer(int fd, char write_buffer[], char read_buffer[] ){ //send 3 times and check return if ok return_ok= 2 or more

                  //---- TEST TRANSFER
//                  char write_buffer[] =" ";
                  int bytes_written = 0;
//                  char read_buffer[32]="";
//                  char *str1,*str2;
                  int bytes_read;
                  printf("MotoCycle!!!\n");
                  int ret; // Retryes 3 times send and listen
                  int return_ok=0; //number of returned OK status from remote device
                  char *istr; //return flag
                  
                  for (ret=0; ret < 14; ret++) //more then 3 times packet check need 10-20
                  {
                   printf("Start Listen:\n");
                   bytes_read = read (fd,&read_buffer,32); //  <<<<---- STUCK
                   printf("read bytes %i\n",bytes_read);
                   printf("recived:[%s]\n",read_buffer);
                   //istr=strstr(str1,str2);
                   istr=strstr(write_buffer,read_buffer);
                     if (istr == NULL)
                         printf("No response\n");
                         else
                         return_ok++;
                         
                   
                   
                   printf("Cycle ReTry counter %i\n",ret);
                   bytes_written= write(fd,write_buffer,sizeof(write_buffer));                  
                   printf("written_bytes in port %i \n",bytes_written);

                  
                  
                  }
return return_ok;
                  
}

int main( int argc, char *argv[] )
{
  int i;
  int fd;
  if (argc > 1)
  {
	  for ( i = 0; i < argc;i++) printf ("%s\n",argv[i]);
	  //fd = open("/dev/ttySP0",O_RDWR | O_NOCTTY);
	  fd = open(argv[1],O_RDWR | O_NOCTTY); // argv[1] first parametr - COM NAME
	 if(fd == -1)
	 {
	   printf("\n  Error! in Opening COM -%s- %i \n",argv[1],fd);
	   
	 }
         if(fd == 1)
                  printf("\n  Error! in Opening %s\n",argv[1]);
                  else
                  printf("\n  %s Opened Successfully %i\n",argv[1],fd);
                  
                  
                  
                  
                  //--- setting for Filestream, COM port speed and reciver wait time
                  int link_up=0; // link state FLAG
                  while (link_up==0)
                  {
                        if(link_up==0) //12 packets send and recive OK
                          {
                           printf("test 600\n");
                            com_setup600(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"a1", "1");                  
                           }
                           
                         if(link_up==0)                         
                          {
                           printf("test 1200\n");
                            com_setup1200(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"a2","1");                  
                           }
                           
                          if(link_up==0)
                          {
                           printf("test 2400\n");
                            com_setup2400(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"a3","1");                  
                           }
                           
                           if(link_up==0)
                          {
                           printf("test 4800\n");
                            com_setup4800(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"f1","1");                  
                           }
                           
                          if(link_up==0)
                          {
                           printf("test 9600\n");
                            com_setup9600(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"f2","1");                  
                           }
                           
                           if(link_up==0)
                          {
                           printf("test 19200\n");
                            com_setup19200(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"f3","1");                  
                           }
                           
                           if(link_up==0)
                          {
                           printf("test 38400\n");
                            com_setup38400(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"f4","1");                                              
                           }
                           
                           if(link_up==0)
                          {
                            printf("test 57600\n");
                            com_setup57600(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"f5","1");                                              
                           }
                           
                          if(link_up==0)
                          {
                           printf("test 115200\n");
                            com_setup115200(fd,33); //file descriptor and wait time in seconds
                            link_up=test_transfer(fd,"f6","1");                                              
                           }
                   printf("Link state [%i] \n",link_up);
                  }


                  
                  
          close(fd);
  }
  
 if (argc == 1)
 {
  printf(" no arguments!!!\n");
  printf("USAGE:\n");
  printf("commands: [port] [boudrate] \n");
  printf(" or \n");
  printf(" for link test: [port] [test] \n");
 }
  

  
  return 0;
}
