#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static const unsigned char sTable[256] ={
  0xa3,0xd7,0x09,0x83,0xf8,0x48,0xf6,0xf4,0xb3,0x21,0x15,0x78,0x99,0xb1,0xaf,0xf9,
  0xe7,0x2d,0x4d,0x8a,0xce,0x4c,0xca,0x2e,0x52,0x95,0xd9,0x1e,0x4e,0x38,0x44,0x28,
  0x0a,0xdf,0x02,0xa0,0x17,0xf1,0x60,0x68,0x12,0xb7,0x7a,0xc3,0xe9,0xfa,0x3d,0x53,
  0x96,0x84,0x6b,0xba,0xf2,0x63,0x9a,0x19,0x7c,0xae,0xe5,0xf5,0xf7,0x16,0x6a,0xa2,
  0x39,0xb6,0x7b,0x0f,0xc1,0x93,0x81,0x1b,0xee,0xb4,0x1a,0xea,0xd0,0x91,0x2f,0xb8,
  0x55,0xb9,0xda,0x85,0x3f,0x41,0xbf,0xe0,0x5a,0x58,0x80,0x5f,0x66,0x0b,0xd8,0x90,
  0x35,0xd5,0xc0,0xa7,0x33,0x06,0x65,0x69,0x45,0x00,0x94,0x56,0x6d,0x98,0x9b,0x76,
  0x97,0xfc,0xb2,0xc2,0xb0,0xfe,0xdb,0x20,0xe1,0xeb,0xd6,0xe4,0xdd,0x47,0x4a,0x1d,
  0x42,0xed,0x9e,0x6e,0x49,0x3c,0xcd,0x43,0x27,0xd2,0x07,0xd4,0xde,0xc7,0x67,0x18,
  0x89,0xcb,0x30,0x1f,0x8d,0xc6,0x8f,0xaa,0xc8,0x74,0xdc,0xc9,0x5d,0x5c,0x31,0xa4,
  0x70,0x88,0x61,0x2c,0x9f,0x0d,0x2b,0x87,0x50,0x82,0x54,0x64,0x26,0x7d,0x03,0x40,
  0x34,0x4b,0x1c,0x73,0xd1,0xc4,0xfd,0x3b,0xcc,0xfb,0x7f,0xab,0xe6,0x3e,0x5b,0xa5,
  0xad,0x04,0x23,0x9c,0x14,0x51,0x22,0xf0,0x29,0x79,0x71,0x7e,0xff,0x8c,0x0e,0xe2,
  0x0c,0xef,0xbc,0x72,0x75,0x6f,0x37,0xa1,0xec,0xd3,0x8e,0x62,0x8b,0x86,0x10,0xe8,
  0x08,0x77,0x11,0xbe,0x92,0x4f,0x24,0xc5,0x32,0x36,0x9d,0xcf,0xf3,0xa6,0xbb,0xac,
  0x5e,0x6c,0xa9,0x13,0x57,0x25,0xb5,0xe3,0xbd,0xa8,0x3a,0x01,0x05,0x59,0x2a,0x46
                                };
                                 
                                  
                                   
unsigned int  Str2Hash (unsigned char *str, unsigned int len)
 {
     unsigned int hash = 0, i;
     unsigned int rotate = 2;
     unsigned int seed = 0x1A4E41U;
                                          
                                           
     for (i = 0; i != len; i++, str++)
         {
                                           
          hash += sTable[(*str + i) & 255];
          hash = (hash << (32 - rotate) ) | (hash >> rotate);
          hash = (hash + i ) * seed;
                                                                  
          }
                                                                               
                                                                        
  return (hash + len) * seed;
  }


unsigned int SimpleHash (unsigned char *str, unsigned int len)
{
  unsigned int seed = 131313; 
  unsigned int hash = 0;
  unsigned int i = 0;
       
         for (i = 0; i < len; str++, i++)
             {
                   hash = (hash * seed) + *str + i;
             }
 return hash;
}


/*
typedef unsigned int HashIndexType;

HashIndexType Hash(char *str) {
    HashIndexType h = 0;
    while (*str) h += *str++;
    return h;
}
*/

typedef unsigned short int HashIndexType;
  
  HashIndexType Hash(int Key) {
      static const HashIndexType K = 40503;
      static const int S = 6;
      return (HashIndexType)(K * Key) >> S;
      }
      
unsigned int xor_tbl[256] = { 0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
                                  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
                                  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
                                  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
                                  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
                                  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
                                  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
                                  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
                                  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
                                  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
                                  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
                                  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
                                  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
                                  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
                                  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
                                  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53 };
                                                                                                                                      
              
int Hash_id(char *test){
int i=0;
int size = strlen(test);
//printf ("SIZE [%i]\n\r",size);
int symbol=0;
int sum=0xffff;
int sum2=0;
unsigned int hash=0;
int cnt=0;

for (i=0; i < size+1; i++)
    {
     symbol = ((int)test[i]); //get ascii code of cyrrent symbol
     if (( symbol > 0 ) && (symbol < 256))
         {
           sum=sum+xor_tbl[symbol]; // replace ascii_code from table crc codes
          }
          
     if  (cnt > 1)
         {
          cnt=0;
          //sum=sum^xor_tbl[symbol]; // replace ascii_code from table crc codes
          sum=sum+sTable[symbol]; // replace ascii_code from table crc codes
         }
     cnt++;
    }

   hash=sum;
return hash;
}

//////////////////

/*
  Name  : CRC-16 CCITT
    Poly  : 0x1021    x^16 + x^12 + x^5 + 1
      Init  : 0xFFFF
        Revert: false
          XorOut: 0x0000
            Check : 0x29B1 ("123456789")
              MaxLen: 4095 байт (32767 бит) - обнаружение
                  одинарных, двойных, тройных и всех нечетных ошибок
                  */
unsigned short Crc16(unsigned char *pcBlock, unsigned short len)
      {
          unsigned short crc = 0xFFFF;
          unsigned char i;
                          
          while (len--)
                {
                  crc ^= *pcBlock++ << 8;
                                          
                  for (i = 0; i < 8; i++)
                      crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
                }
  return crc;
}

///////////////////

/*

int Hash_id2(char *test){
int i=0;
int size = strlen(test);
int symbol=0;
int sum=0;
unsigned int hash=0;
int cnt=0; //symbol counter
char *buf;
for (i=0; i < size+1; i++)
    {
     symbol = ((int)test[i]); //get ascii code of cyrrent symbol
     if (( symbol > 0 ) && (symbol < 256)) //filter of code symbols
         {
          if ( cnt < 10 )
           sum=sum+xor_tbl[symbol]; // replace ascii_code from table crc8 codes
           if  ( cnt > 10 )  buf=buf+test[i];
          }    
     cnt++;
    }
   
   hash=sum;
   sum = Crc16(buf,strlen(buf));
   hash=hash+sum;
return hash;
}
*/




/*
#define MULTIPLER 31
unsigned int hash1char (char *str, int len){
unsigned h;
int n =0;
for (n=0; n < len; n++){
  h = MULTIPLER * h + (unsigned char)str[n]; // приводят к беззнаковому, чтобы итоговое число было положительное 
  }
  h = h % len;
return h;
}
*/

int scrambler (char str[45], char result[45]){

int len = strlen(str);
int i=0;
int cnt=0;
    if ( len > 45 ) return -1;
    
    for (i=0; i < len; i++){
         result[i]=str[i];
         if ( cnt > 1 ) result[i]=str[i-1];
         cnt++;
        }

return 0;
}

int main ()
{
 char test22[60]="Test";
 char *istr;
 char sep[2] =",";
 unsigned int res,res2;
 FILE * ptrFile = fopen("signals.cfg" , "r");
 char mystring [100];
     
        if (ptrFile == NULL) perror("Error opening file\n\r");
           else
              {
              while ( fgets(mystring, 100, ptrFile) != NULL ) {// считать символы из файла
                     //puts(mystring);  // вывод на экран
                       istr = strtok(mystring,sep);
                     if (istr != NULL){                         
                         // res2 = Hash_id2(istr);
                         //char str[55];
                         //scrambler (istr,str);
                         //printf ("%s %s \n\r",istr,str);
                         
                         
                         res = Crc16(istr,strlen(istr));
                         //res = Crc16(str,strlen(str));
                         
                         //res = Str2Hash(istr,strlen(istr));
                         //res2 = SimpleHash(istr,strlen(istr));
                         //res2 = hash1char (istr,strlen(istr));
                         
                         
                         printf ("%s %i \n\r",istr,res);
                         printf ("%i \n\r",res);
                      }
               }
               fclose (ptrFile);
              }
 return 0;
}
