#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "/home/opc/Kombain/test/include/modbus/modbus.h"
#include "network.h"
int main(int argc, char *argv[]){
modbus_t *mb;
uint16_t tab_reg[32];
uint16_t wr_reg[32];
int rc;
int i;
    
mb = modbus_new_tcp("192.168.1.150", 502);

        if (modbus_connect(mb) == -1) {
            fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
            modbus_free(mb);
            return -1;
        }
         
if (mb == NULL) {
    fprintf(stderr, "Unable to allocate libmodbus context\n");
    return -1;
   }
        

/* Read 5 registers from the address 0 */
/*
modbus_read_registers(mb, 0, 31, tab_reg);
int i=0;
for (i = 0; i < 32; i++){
     printf("reg_%i[%i] \n\r",i,tab_reg[i]);
}
*/       
        /* 
        socket_init();
        tcpsignal_read(".");
        socket_close();
        */
        printf("Wago Server Module");
        rc = modbus_read_registers(mb, 0, 32, tab_reg);
        if (rc == -1) {
            fprintf(stderr, "mb_rd: %s\n", modbus_strerror(errno));
            return -1;
            }

           for (i=0; i < rc; i++) {
                printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
           }
           
// 518 - bitmask relay 16bit  6 relay from lower byte 
// 6 - inputs cords, pedals
// 1 - voltage

    wr_reg[0] = 0x3f;
    rc = modbus_write_registers(mb, 36, 65000, wr_reg);
        if (rc == -1) {
            fprintf(stderr, "mb_wr: %s\n", modbus_strerror(errno));
            return -1;
            }
                    
modbus_close(mb);
modbus_free(mb);               

return 0;
}