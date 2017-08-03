#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "/home/opc/Kombain/test/include/modbus/modbus.h"

int main(int argc, char *argv[]){
int slava;
uint16_t tab_reg[32];
// 'E' - event work mode, 8 - data bits, 1 - stop bit - serial port configuration
modbus_t* my_modbus = modbus_new_rtu("/dev/ttySP0", 115200, 'N', 8, 1);

slava=modbus_set_slave(my_modbus, 35);

printf("Slavik sdoh %i\n",slava);
modbus_connect(my_modbus);
modbus_read_registers(my_modbus, 0, 8, tab_reg);
int i=0;
               
               for (i=0;i<8;i++)
               {
                printf ("Mordor %i\n",tab_reg[i]);
               }
modbus_close(my_modbus);
modbus_free(my_modbus);
return 0;
}