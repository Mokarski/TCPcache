#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dState=0; //debug state

int load_signals(){
return 0;
}

int find_interfaces(){
retrn 0;
}

int test_interfaces(){
return 0;
}


void show_log(char *msg){
}

void write_journal(char *msg){
}



int debug_state (int state){

switch ( state ){
case 1:
       show_log();
       write_journal();
       break;
       
case 2:
       show_log();
       write_journal();       
       break;
       
default:
       printf(" no error \n\r");
}

return 0;
}
