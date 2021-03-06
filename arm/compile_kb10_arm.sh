#LD_LIBRARY_PATH=/home/opc/Kombain/libmodbus-2.9.2
#export LD_LIBRARY_PATH
#echo  $LD_LIBRARY_PATH

echo "hash"
#gcc -Wall -g -c network.c
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c hash.c   -o  hash_arm.o

echo "network"
#gcc -Wall -g -c network.c
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c network.c   -o  network_arm.o

echo "speedtest"
#gcc -Wall -g -c speedtest.c
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c speedtest.c   -o  speedtest.o

echo "signals"
#gcc -Wall -g -c signals.c
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c signals.c   -o  signals_arm.o

echo "network"
#gcc -Wall -g -c network.c
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c network.c   -o  network_arm.o 

#echo "virtmb"
#gcc -Wall -g -c virtmb.c
#/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c  virtmb.c   -o   virtmb_arm.o

#gcc -Wall -g  -c cashserver.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o server.o

echo "preassembly assembly"
#gcc -Wall -g  -c client_pcsens.c 
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -c   client_kb10.c    -o   client_kb10_arm.o

#gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread  -o server server.o signals.o

#gcc -Wall -g -c client.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o client.o
#gcc -o client client.o signals.o 

echo "final assembly"
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc   -o client_kb10_arm client_kb10_arm.o signals_arm.o  speedtest.o network_arm.o hash_arm.o
#gcc -Wall -g  $(pkg-config --libs --cflags /home/opc/Kombain/libmodbus-3.0.6/libmodbus.pc) -o client_pcsens client_pcsens.o signals.o virtmb.o
#./client_pcsens
