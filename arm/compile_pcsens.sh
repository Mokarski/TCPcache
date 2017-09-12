LD_LIBRARY_PATH=/home/opc/Kombain/libmodbus-2.9.2
export LD_LIBRARY_PATH
echo  $LD_LIBRARY_PATH

#/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc  -o $2 $1

#/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc --help
#arm-linux-gnueabi-gcc main.c $(pkg-config --libs --cflags ~/BBB/build/lib/pkgconfig/libmodbus.pc)

#gcc -Wall -g -c network.c

#gcc -Wall -g -c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread $(pkg-config --libs --cflags /home/opc/Kombain/libmodbus-3.0.6/libmodbus.pc) test_wago.c -o wago.o

#gcc  -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread $(pkg-config --libs --cflags /home/opc/Kombain/libmodbus-3.0.6/libmodbus.pc) wago_net wago.o network.o

#gcc $1 -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread $(pkg-config --libs --cflags /home/opc/Kombain/libmodbus-3.0.6/libmodbus.pc) -o $2 wago.o network.o


#========================================================================

#rm *.o

gcc -Wall -g -c signals.c

gcc -Wall -g -c virtmb.c
#gcc -Wall -g  -c cashserver.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o server.o

gcc -Wall -g  -c client_pcsens.c 
#gcc -Wall -g  pc_sens.c -o pc_sens


#gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread  -o server server.o signals.o

#gcc -Wall -g -c client.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o client.o
#gcc -o client client.o signals.o 


gcc -Wall -g  $(pkg-config --libs --cflags /home/opc/Kombain/libmodbus-3.0.6/libmodbus.pc) -o client_pcsens client_pcsens.o signals.o virtmb.o network.o
#./client_pcsens
