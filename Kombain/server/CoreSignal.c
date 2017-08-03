#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#define MAX 100
#define STR_LEN_TXT 100

struct timeval tv1,tv2,dtv;
struct timezone tz;

void time_start() {
gettimeofday(&tv1,&tz);
}

long time_stop(){
gettimeofday(&tv2,&tz);
dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
   if(dtv.tv_usec<0) 
   { 
     dtv.tv_sec--; dtv.tv_usec+=1000000; 
   }
 return dtv.tv_sec*1000+dtv.tv_usec/1000;
}

struct Signal {
 char Name[100];       //Name signal
 int  MB_Id;           //Modbus device ID
 int  MB_Reg_Num;      //Mb register number
 char Val_Type[10];    //type of value int or bit 
 int  Value[2];        //value 2 int 
  char Reserv1[10];    //type of value int or bit
  char Reserv2[10];    //type of value int or bit
  char Reserv3[10];    //type of value int or bit
  int Off;            //Signal not used if OFF = 1;
  int ExState;        // 1-for execution,2-executed,3-expired,4-dropped 
  int Ex_HW;   //Execution Hardware mark PcProc =1 Panel 43=2 Panel 10=3 | soft_class 
  int Ex_SF;   //Execution Software mark ModBusMaster_RTU =1 Modbus_Master_TCP=2 CoreSignal=100
  int Prio;    //Priority of signal 0 - no priority
}  Signal_Array [MAX];

void init_list(void)
{
 register int t;
 for(t=0; t<MAX; ++t) Signal_Array[t].Name[0] = '\0';
}

void list (void)
{
register int t;
for (t=0; t<MAX; ++t) {
if(Signal_Array[t].Name[0]) 
  {
   printf("Signal Name: %s\n", Signal_Array[t].Name);
   Signal_Array[t].MB_Id=atoi(Signal_Array[t].Reserv1);
   printf("MB ID: %i\n", Signal_Array[t].MB_Id);
   
   Signal_Array[t].MB_Reg_Num=atoi(Signal_Array[t].Reserv2);
   printf ("MB Register Num: %i\n", Signal_Array[t].MB_Reg_Num);
   
   printf ("Value type: %s\n", Signal_Array[t].Val_Type);
//     printf ("Reserve1: %s\n", Signal_Array[t].Reserv1); //reserved for file input conversion
//     printf ("Reserve2: %s\n", Signal_Array[t].Reserv2);
//     printf ("Reserve3: %s\n", Signal_Array[t].Reserv3);
     
     printf ("Signal OFF: %i\n", Signal_Array[t].Off);
     printf ("Execution flow state: %i\n", Signal_Array[t].ExState);
     printf ("Hardware execution mark: %i\n", Signal_Array[t].Ex_HW);
     printf ("Software execution mark: %i\n", Signal_Array[t].Ex_SF);
     printf ("Priority mark: %i\n", Signal_Array[t].Prio);
   Signal_Array[t].Value[0]=atoi(Signal_Array[t].Reserv3);
   printf ("Values: [Hi: %i] [Low: %i]\n\n", Signal_Array[t].Value[0],Signal_Array[t].Value[1]);
  }
}
printf("\n\n");
}

//kb.kbl.stop_oil_pump, bit, 31, 1, 0

int text_parser(char *str_in, char str_out1[STR_LEN_TXT], char str_out2[STR_LEN_TXT], char str_out3[STR_LEN_TXT], char str_out4[STR_LEN_TXT], char str_out5[STR_LEN_TXT])
{
 int i=0;
  i=strlen (str_in);
 int x=0;
 int block1=0; //block counter
 int block2=0;
 int block3=0;
 int block4=0;
 int block5=0;

 int  complete1=0; //flag to fill block
 int  complete2=0;
 int  complete3=0;
 int  complete4=0;
 int  complete5=0;
 
// printf("START PARSER \n");
 while ( x < i ) 
 {
    if ( (str_in[x]!=',') && (complete1 == 0) )
      {
        str_out1[block1]=str_in[x];
        block1++;
      }else       
      {
        if ( (str_in[x]==',') && (complete1 == 0) )
        {
          x++; //add +1 in counter etc next symbol
          complete1=1;
//          printf ("string str_out1 %s\n",str_out1);  
        }
      }
      
// printf ("complete1 %i\n",complete1);
      if ((complete1 == 1) && (complete2 == 0))
      {
    //   printf("Blohh 2\n");
        if (str_in[x]!=',')
         {
    //      printf("INPUT %c \n",str_in[x]);
          str_out2[block2]=str_in[x];
          block2++;
         }else
         {
          if (str_in[x]==',')
          {
           x++; //add +1 in counter etc next symbol
           complete2=1;
//           printf ("str_out2 %s\n",str_out2);
          }
         }
      
      }

      if ((complete2 == 1) && (complete3 == 0))
      {
        if (str_in[x]!=',')
         {
          str_out3[block3]=str_in[x];
          block3++;
         }else
         {
          if (str_in[x]==',')
          {
           x++; //add +1 in counter etc next symbol
           complete3=1;
//           printf ("str_out3 %s\n",str_out3);
          }
         }
      
      }


      if ((complete3 == 1) && (complete4 == 0))
      {
        if (str_in[x]!=',')
         {
          str_out4[block4]=str_in[x];
          block4++;
         }else
         {
          if (str_in[x]==',')
          {
           x++; //add +1 in counter etc next symbol
           complete4=1;
//           printf ("str_out4 %s\n",str_out4);
          }
         }
      
      }
      
      if ((complete4 == 1) && (complete5 == 0))
      {
        if (str_in[x]!=',')
         {
          str_out5[block5]=str_in[x];
          block5++;
         }else
         {
          if (str_in[x]==',')
          {
           //x++; //add +1 in counter etc next symbol
           complete5=1;
//           printf ("str_out5 %s\n",str_out5);
          }
          if (str_in[x]=='\0')
          {
           //x++; //add +1 in counter etc next symbol
           complete5=1;
//           printf ("str_out5 %s\n",str_out5);
          }
         }
      
      }

      
  x++;
 }
// printf("END PARSER \n");
//       printf ("string str_out1 %s\n",str_out1);  
//       printf ("string str_out2 %s\n",str_out2);  
//       printf ("string str_out3 %s\n",str_out3);  
//       printf ("string str_out4 %s\n",str_out4);  
//       printf ("string str_out5 %s\n",str_out5);  

return 0;
}

int file_load(void )
{
 FILE *fp;
 char str[128];
 char *row;
 int c;
  printf ("Load signals from signals.cfg\n");
 fp = fopen ("signals.cfg","r");
 if (fp == NULL) {printf ("error open file\n"); return -1;}
 else printf("Read from file next signals:\n");
    c=0;
 
 while (1)
 {
   row = fgets (str,sizeof(str),fp);
   if (row == NULL)
     {
      if ( feof (fp) !=0 )
      {
       printf ("Read signals from file,complete.\n");
       break;
      }
      else
      {
       printf ("\n ERROR: read from file\n");
       break;
      }
     }
    printf ("    %s",str); //string from file
    text_parser(str,Signal_Array[c].Name,Signal_Array[c].Val_Type,Signal_Array[c].Reserv1,Signal_Array[c].Reserv2,Signal_Array[c].Reserv3);
    //strcpy(Signal_Array[c].Name,str);
    c++;
  }
 printf ("Close file: ");
 if ( fclose(fp) == EOF) printf ("error close file \n");
    else printf ("complete\n");
    

  printf ("Loaded Signals: \n");
 
return 0;
}

int signal_update (char sName[100],int sVal){ //function to update parameters in tcp_cache_server list
int n=0;
int cnt=0;
  for (n=0;n<MAX;n++)
  {
   if (strstr(Signal_Array[n].Name,sName) != NULL) 
      {
        Signal_Array[n].Value[1]=sVal;
        cnt++;
      } 
  }
 return cnt; //if zerro then not found signals else number of matches
}

int main ()
{
 time_start();
 init_list();
 file_load();
 list(); //fill signals and echo to screen
 printf("Time: %ld ms\n", time_stop());
 time_start();
 printf ( "Found signals: %i\n\n",signal_update("kb.kei3.telescope_down",100));
 list();
 printf("Time: %ld ms\n", time_stop());
return 0;
}
