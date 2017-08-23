#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 /* reverse:  переворачиваем строку s на месте */
  void reverse(char s[])
   {
        int i, j;
             char c;
              
                   for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
                            c = s[i];
                            s[i] = s[j];
                            s[j] = c;
                       }
    }

 /* itoa:  конвертируем n в символы в s */
  void itoa(int n, char s[])
   {
        int i, sign;
         
              if ((sign = n) < 0)  /* записываем знак */
                  n = -n;          /* делаем n положительным числом */
              i = 0;
              do {      
                        /* генерируем цифры в обратном порядке */
                        s[i++] = n % 10 + '0';   /* берем следующую цифру */
                        
                 } while ((n /= 10) > 0);     /* удаляем */
             if (sign < 0)
                 s[i++] = '-';
             s[i] = '\0';
  reverse(s);
  }

void init_signals_list(void)
{
 register int t;
  for(t=0; t<MAX_Signals; ++t) Signal_Array[t].Name[0] = '\0';
 }


int signal_read(char sName[100]){
int n=0;
int result=0;
  for (n=0;n<MAX_Signals;n++)
    {
       if (strstr(Signal_Array[n].Name,sName) != NULL)
             {
                     result=Signal_Array[n].Value[1];
                 break;
             }
     }
  return result; //if zerro then not found signals else return value
}

int signals_txt(){
int n=0;
int result=0;
char Value[3];
//int signals_counter=0;
  for (n=0;n<MAX_Signals;n++)
      {
             if (strlen(Signal_Array[n].Name) > 4 ) //if name signal more then 4 symbols
                 {
                 //strcat(packed_txt_string,"start_set_signals:");
                 strcat(packed_txt_string,Signal_Array[n].Name);
                 strcat(packed_txt_string,":");
                 itoa(Signal_Array[n].Value[1],Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,";");                 
                               
                 result++;                                            
//                 signasl_counter = result;
                 }
      }
 return result; //if zerro then not found signals else return value
}
                                                                                    

int unpack_signal (char *str, int n){
    char sep[3]=":";
    char *istr;
    //printf("Unpack Signal: [%s]\n\r",str);

istr = strtok (str, sep);
if (istr != NULL){
   //printf ("1 - %s ",istr);
   int c =1;
    while ( istr != NULL ){
           switch (c) {
           case 1:
                  //printf ("%i - %s ",c,istr);
                  strcpy ( Signal_Array[n].Name,istr );
           break;

           case 2:
                 //printf ("%i - %s ",c,istr);
                 strcpy ( Signal_Array[n].Val_Type,istr );
           break;

           case 3:
                 //printf ("%i - %s ",c,istr);
                  Signal_Array[n].MB_Id = atoi ( istr );
           break;

           case 4:
                 //printf ("%i - %s ",c,istr);
                 Signal_Array[n].MB_Reg_Num = atoi ( istr );
           break;

           case 5:
                 //printf ("%i - %s ",c,istr);
                 Signal_Array[n].Bit_Pos = atoi ( istr );
           break;

           case 6:
                 //printf ("%i - %s ",c,istr);
                 Signal_Array[n].Value[1] = atoi ( istr );
           break;

           case 7:
                 //printf ("%i - %s ",c,istr);
           break;

           case 8:
                 //printf ("%i - %s ",c,istr);
           break;

           case 9:
                 //printf ("%i - %s ",c,istr);
           break;

           case 10:
                 //printf ("%i - %s ",c,istr);
           break;

           case 11:
                 //printf ("%i - %s \n\r",c,istr);
           break;

           default:
           printf("error index %i\n\r",c);
           }

           c++;
           istr = strtok (NULL, sep);
           //printf ("%i - %s ",c,istr);
          }
}
return 0;
}


                
int sDeSerial_by_num (int n){
char sep1[10]=":";
//char sep2[10]=";";
char line[503]; //buffer for parsing
char *istr1;
//char *istr2;
//char *istr3;
           if (strlen (Signal_Array[n].Name) < 2){
            printf("SignalName very shot <2 symbols \n\r");
            return 1;
           }

           //erase buffer
           line[0]=0;
           
          //explode signal to name field and val field
          strcpy(line,Signal_Array[n].Name); //from filed Name content all signals info to buf
          printf ("[#%i]==> buffer line:[%s]\n\r",n,line); //DEBUG
          
          //strcpy(line,packed_txt_string); //from filed Name to buf
          
          istr1 = strtok (line,sep1); //extract Name field

          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok1\n\r");
          return 1;
          }
                    
          printf("\n\rDS:> Signal Name:{%s} ",istr1);
          strcpy (Signal_Array[n].Name,istr1); //put "Name" to filed Name
          
          
          
          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok2\n\r");
          return 1;
          }
          strcpy (Signal_Array[n].Val_Type,istr1);
          printf("Value Type:{%s} ",istr1);
                                                
                                                                  
          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok3\n\r");
          return 1;
          }
          //printf ("[#%i] MB_ID:[%s]\n\r",n, istr1); //DEBUG
          Signal_Array[n].MB_Id = atoi (istr1);                                                                                                              
          printf( " MB_Id:{%s} ",istr1);

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok4\n\r");
          return 1;
          }
          Signal_Array[n].MB_Reg_Num = atoi (istr1);                                                                                                              
          printf(" RegNum:{%s} ",istr1);

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok5\n\r");
          return 1;
          }
          Signal_Array[n].Bit_Pos = atoi (istr1);                                                                                                              
          printf(" BitPos:{%s} ",istr1);
        
          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok6\n\r");
          return 1;
          }
          int val=0;
          val = atoi (istr1);              
          // 48                                                                                                
          Signal_Array[n].Value[1] = val;
          printf(" Value1:[ str %s | int %i ] ",istr1, val);

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok7\n\r");
          return 1;
          }
          
          Signal_Array[n].TCP_Type = atoi (istr1);                                                                                                                                                                                                                                              

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok8\n\r");
          return 1;
          }
          strcat(Signal_Array[n].TCP_Addr, istr1);                                                                                                              

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok9\n\r");
          return 1;
          }
          Signal_Array[n].Prio = atoi (istr1);                                                                                                              

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok10\n\r");
          return 1;
          }
          Signal_Array[n].Off = atoi (istr1);  //state On/Off                                                                                                             
          
          /*
          istr1 = strtok (NULL,sep1); // ?????????
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok11\n\r");
          return 1;
          }
          Signal_Array[n].ExState = atoi (istr1);                                                                                                              
          */
return 0;
}

int sSerial_by_num(int n){
//int n=0;
int result=0;
char Value[3];
//int signals_counter=0;

             if (strlen(Signal_Array[n].Name) > 2 ) //if name signal more then 4 symbols
                 {
                 
                 strcat(packed_txt_string,Signal_Array[n].Name);
                 strcat(packed_txt_string,":");
                 
                 strcat(packed_txt_string,Signal_Array[n].Val_Type);
                 strcat(packed_txt_string,":");                 
                 
                 itoa(Signal_Array[n].MB_Id,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].MB_Reg_Num,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].Bit_Pos,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 
                 itoa(Signal_Array[n].Value[1],Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 

                 itoa(Signal_Array[n].TCP_Type,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 strcat(packed_txt_string,Signal_Array[n].TCP_Addr);
                 strcat(packed_txt_string,":");
                 

                 itoa(Signal_Array[n].Prio,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].Off,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].ExState,Value);
                 strcat(packed_txt_string,Value);
                 //strcat(packed_txt_string,":");
                 
                 strcat(packed_txt_string,";");                 
                               
                 result++;                                            
//                 signasl_counter = result;
                 }
      
 return result; //if zerro then not found signals else return value
}
                


int sDeSerial_by_num_short (int n){
char sep1[10]=":";
char sep2[10]=";";
char line[503]; //buffer for parsing
char *istr1;
char *istr2;
char *istr3;

       //explode signal to name field and val field
          strcpy(line,Signal_Array[n].Name); //from filed Name to buf
//          printf ("[#%i] line:[%s]\n\r",n,line); //DEBUG
          
          //strcpy(line,packed_txt_string); //from filed Name to buf
          
          istr1 = strtok (line,sep1); //extract Name field

          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok1\n\r");
          return 1;
          }
                    
          strcpy (Signal_Array[n].Name,istr1); //put "Name" to filed Name
          
          
          
          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok2\n\r");
          return 1;
          }
          strcpy (Signal_Array[n].Val_Type,istr1);
                                                
                                                                  
          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok3\n\r");
          return 1;
          }
          //printf ("[#%i] MB_ID:[%s]\n\r",n, istr1); //DEBUG
          Signal_Array[n].MB_Id = atoi (istr1);                                                                                                              


          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok4\n\r");
          return 1;
          }
          Signal_Array[n].MB_Reg_Num = atoi (istr1);                                                                                                              


          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok5\n\r");
          return 1;
          }
          Signal_Array[n].Bit_Pos = atoi (istr1);                                                                                                              
        
        
          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok6\n\r");
          return 1;
          }
          Signal_Array[n].Value[1] = atoi (istr1);                                                                                                              


          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok7\n\r");
          return 1;
          }
          Signal_Array[n].TCP_Type = atoi (istr1);                                                                                                                                                                                                                                              

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok8\n\r");
          return 1;
          }
          strcat(Signal_Array[n].TCP_Addr, istr1);                                                                                                              

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok9\n\r");
          return 1;
          }
          Signal_Array[n].Prio = atoi (istr1);                                                                                                              

          istr1 = strtok (NULL,sep1);
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok10\n\r");
          return 1;
          }
          Signal_Array[n].Off = atoi (istr1);  //state On/Off                                                                                                             
          
          /*
          istr1 = strtok (NULL,sep1); // ?????????
          if (istr1 == NULL)  {
          printf ("DeSerializer: Null strtok11\n\r");
          return 1;
          }
          Signal_Array[n].ExState = atoi (istr1);                                                                                                              
          */
return 0;
}


int sSerial_by_num_short(int n){
//int n=0;
int result=0;
char Value[3];
//int signals_counter=0;

             if (strlen(Signal_Array[n].Name) > 3 ) //if name signal more then 4 symbols
                 {
                 
                 strcat(packed_txt_string,Signal_Array[n].Name);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].Value[1],Value);
                 strcat(packed_txt_string,Value);
                 
                 //strcat(packed_txt_string,":");
                 
                 /*
                 strcat(packed_txt_string,Signal_Array[n].Val_Type);
                 strcat(packed_txt_string,":");                 
                 
                 itoa(Signal_Array[n].MB_Id,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].MB_Reg_Num,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].Bit_Pos,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 
                 itoa(Signal_Array[n].Value[1],Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 

                 itoa(Signal_Array[n].TCP_Type,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 strcat(packed_txt_string,Signal_Array[n].TCP_Addr);
                 strcat(packed_txt_string,":");
                 

                 itoa(Signal_Array[n].Prio,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].Off,Value);
                 strcat(packed_txt_string,Value);
                 strcat(packed_txt_string,":");
                 
                 itoa(Signal_Array[n].ExState,Value);
                 strcat(packed_txt_string,Value);
                 //strcat(packed_txt_string,":");
                 */
                 strcat(packed_txt_string,";");                 
                               
                 result++;                                            
//                 signasl_counter = result;
                 }
      
 return result; //if zerro then not found signals else return value
}
                


//  update Signal value and execution state
int signal_update_ex (char sName[100],int sVal, int Execution_State){ //function to update parameters in signals array
int n=0;
int cnt=0;
  for (n=0; n < MAX_Signals; n++)
    {
       if (strstr(Signal_Array[n].Name,sName) != NULL)
             {
                     Signal_Array[n].Value[1]=sVal;
                     Signal_Array[n].ExState=Execution_State;
                     cnt++;
                     break;
               }
   }
  return cnt; //if zerro then not found signals else number of matches
}
                                                      

void block_all_signals(void)
{
   register int t;
    for(t=0; t<MAX_Signals; ++t)
      {
      // Signal_Array[t].Name[0] = '\0';
         Signal_Array[t].Off = 1; //signal marked as off
           }
}
               
void unblock_all_signals(void)
{
    register int t;
         for(t=0; t<MAX_Signals; ++t)
             {
               //   Signal_Array[t].Name[0] = '\0';
               Signal_Array[t].Off = 0; //signal marked as off
             }
}
                    
                        
void print_signals_list (void) //print all signals
{
register int t;
for (t=0; t<MAX_Signals; t++) {
if(Signal_Array[t].Name[0])
  {
       printf("Signal Name: %s\n", Signal_Array[t].Name);
       printf ("Value type: %s\n", Signal_Array[t].Val_Type);
       
       
       Signal_Array[t].MB_Id=atoi(Signal_Array[t].Reserv1);
       printf("MB ID: %i\n", Signal_Array[t].MB_Id);
           
       Signal_Array[t].MB_Reg_Num=atoi(Signal_Array[t].Reserv2);
       printf ("MB Register Num: %i\n", Signal_Array[t].MB_Reg_Num);
       
       Signal_Array[t].Bit_Pos=atoi(Signal_Array[t].Reserv3);
       printf ("MB Register BitNum: %i\n", Signal_Array[t].Bit_Pos);
      
       printf("TCP_Type: %i\n ", Signal_Array[t].TCP_Type);
       printf("TCP_Addr: %s\n ", Signal_Array[t].TCP_Addr);
       
       printf ("Values: [Hi: %i] [Low: %i]\n\n", Signal_Array[t].Value[0],Signal_Array[t].Value[1]);
     
                          //     printf ("Reserve1: %s\n", Signal_Array[t].Reserv1); //reserved for file input conversion
                          //     printf ("Reserve2: %s\n", Signal_Array[t].Reserv2);
                          //     printf ("Reserve3: %s\n", Signal_Array[t].Reserv3);
       printf ("Priority mark: %i\n", Signal_Array[t].Prio);                          
       printf ("Signal OFF: %i\n", Signal_Array[t].Off);
       printf ("Execution  state: %i\n", Signal_Array[t].ExState);
       printf ("Hardware execution mark: %i\n", Signal_Array[t].Ex_Hw);
       printf ("Software execution mark: %s\n", Signal_Array[t].Ex_Sf);
       
                                   
      
    }
   }
    printf("\n\n");
}
                                                        

void print_by_name (char sName[100]) //print all signals
{
register int t;
int res=0;
for (t=0; t<MAX_Signals; t++) {
  if( strstr(Signal_Array[t].Name,sName) )
    {
       printf("Signal Name: %s\n", Signal_Array[t].Name);
       printf ("Value type: %s\n", Signal_Array[t].Val_Type);
       printf("MB ID: %i\n", Signal_Array[t].MB_Id);
       printf ("MB Register Num: %i\n", Signal_Array[t].MB_Reg_Num);
       printf ("MB Register BitNum: %i\n", Signal_Array[t].Bit_Pos);
       printf("TCP_Type: %i\n ", Signal_Array[t].TCP_Type);
       printf("TCP_Addr: %s\n ", Signal_Array[t].TCP_Addr);
       printf ("Values: [Hi: %i] [Low: %i]\n\n", Signal_Array[t].Value[0],Signal_Array[t].Value[1]);
       printf ("Priority mark: %i\n", Signal_Array[t].Prio);                          
       printf ("Signal OFF: %i\n", Signal_Array[t].Off);
       printf ("Execution  state: %i\n", Signal_Array[t].ExState);
       printf ("Hardware execution mark: %i\n", Signal_Array[t].Ex_Hw);
       printf ("Software execution mark: %s\n", Signal_Array[t].Ex_Sf);
       res++;
       break; // break cycle
     }
   }
    if ( res==0 ) printf("Not found signal with this signal NAME\n\n");
}
                                                                                 


int signals_file_load(void )
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
 for (n=0;n<MAX_Signals;n++)
    {
       if (strstr(Signal_Array[n].Name,sName) != NULL)
             {
                     Signal_Array[n].Value[1]=sVal;
                             cnt++;
                                     break;
             }
     }
 return cnt; //if zerro then not found signals else number of matches
}
                                              
                                              


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
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               


