#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define Module_255 255
int Hash_id(char *test){
int i=0;
int size = strlen(test);
int symbol=0;
int sum=0;
int hash=0;
if ( size > 0 )
 {
   for (i=0; i < size; i++){
        symbol = ((int)test[i]-(int)'0');
        sum=(sum+symbol)%Module_255; //addiction by module N
    }
   hash=(sum+size);
   printf ("strln{%i} Hash[%i]  Sum[%i] SUM6>>[%i]\n\r",size,hash,sum ,sum >> 6);
  }
return hash;
}

