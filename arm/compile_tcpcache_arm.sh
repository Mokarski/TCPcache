#rm *.o

#gcc -Wall -g -c signals.c


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



/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc  -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread -c cashserver.c -o tcpcache.o
#gcc -Wall -g  -c cashserver.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o server.o


/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread  -o  tcpcache signalhash.c tcpcache.o signals_arm.o speedtest.o network_arm.o
#gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread  -o server server.o signals.o
echo DONE


