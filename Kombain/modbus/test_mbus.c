#include <stdio.h>
#include <stdlib.h>
#include <modbus.h>

int main(int argc, char *argv[]){
modbus_t *plc_client;

plc_client = modbus_new_tcp_pi("192.168.1.230","502");
if (plc_client == NULL) {
    fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
        }
        if (modbus_connect(plc_client) == -1) {
            fprintf(stderr, "Connection failed: \n");
                modbus_free(plc_client);
                    return -1;
                    }
                    else if(modbus_connect(plc_client) == 0) {
                        printf("MODBUS CONNECTION SUCCESSFUL\n");
                        }
                        
                        uint8_t* catcher = malloc(sizeof(uint8_t));
                        
                        if(modbus_read_bits(plc_client, 2000, 1, catcher)>0){
                            printf("READ SUCCESSFUL");
                            }
                            else{
                                printf("READ FAILED");
                                }
                                
                                free(catcher);
                                modbus_close(plc_client);
                                modbus_free(plc_client);
                                
                                return 0;
                                }