#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "/home/opc/Kombain/libmodbus-2.9.2/src/modbus.h"

int main(int argc, char *argv[]){
uint16_t tab_reg[32];
// 'E' - event work mode, 8 - data bits, 1 - stop bit - serial port configuration
modbus_t* my_modbus = modbus_new_rtu("/dev/ttyUSB0", 57600, 'E', 8, 1);
modbus_set_slave(my_modbus, 0x0B);
modbus_connect(my_modbus);
modbus_read_registers(my_modbus, 0, 5, tab_reg);
modbus_close(my_modbus);
modbus_free(my_modbus);
               
return 0;
}