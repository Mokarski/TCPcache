#cc -Wall -g test_mbus.c -o modbus -L /usr/local/lib -lmodbus
LD_RUN_PATH=/usr/local/lib
export LD_RUN_PATH
LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH


#gcc test_mbus.c -o mbtest `-Wl,-rpath -Wl,LIBDIR -LLIBDIR pkg-config --libs --cflags libmodbus`
#gcc -I/</home/opc/Kombain/libmodbus-2.9.2/src/> -c test_mbus.c -Wl,-rpath=/usr/local/lib -Wl,LIBDIR -o com.o
#gcc -I /usr/include/modbus test_mbus.c -o myprogram -lmodbus -lpthread

gcc -L/usr/local/lib -I/usr/include/modbus test_mbus.c -o mbtest `pkg-config --libs --cflags libmodbus`