#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 100
#define STR_LEN_TXT 100

struct Signal {
 char Name[100];      //Name signal
 int MB_Id;           //Modbus device ID
 int MB_Reg_Num;      //Mb register number
 char Val_Type[10];   //type of value int or bit 
 int  Value[2];       //value 2 int 
  char Reserv1[10];   //type of value int or bit
  char Reserv2[10];   //type of value int or bit
  char Reserv3[10];   //type of value int or bit
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
   printf("MB ID: %i\n", Signal_Array[t].MB_Id);
   printf ("MB Register Num: %i\n", Signal_Array[t].MB_Reg_Num);
   printf ("Value type: %s\n", Signal_Array[t].Val_Type);
     printf ("Reserve1: %s\n", Signal_Array[t].Reserv1);
     printf ("Reserve2: %s\n", Signal_Array[t].Reserv2);
     printf ("Reserve3: %s\n", Signal_Array[t].Reserv3);
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
 
 printf("START PARSER \n");
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
          complete1=1;
          printf ("string str_out1 %s\n",str_out1);  
        }
      }
      
// printf ("complete1 %i\n",complete1);
      if ((complete1 == 1) && (complete2 == 0))
      {
       printf("Blohh 2\n");
        if (str_in[x]!=',')
         {
          printf("INPUT %c \n",str_in[x]);
          str_out2[block2]=str_in[x];
          block2++;
         }else
         {
          if (str_in[x]==',')
          {
           complete2=1;
           printf ("str_out2 %s\n",str_out2);
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
           complete3=1;
          }
         }
      
      }


      
  x++;
 }
 printf("END PARSER \n");
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
 if ( fclose(fp) ==EOF) printf ("error \n");
    else printf ("complete\n");
    

  printf ("Loaded Signals: \n");
 
return 0;
}

int main ()
{
 init_list();
 file_load();
 list();
return 0;
}
