#rm *.o

#gcc -Wall -g -c signals.c
echo precomplie
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc  -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread -c test_srv.c -o test_srv.o
#gcc -Wall -g  -c cashserver.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o server.o

echo compile
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread  -o  test_srv test_srv.o
#gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread  -o server server.o signals.o

echo DONE


