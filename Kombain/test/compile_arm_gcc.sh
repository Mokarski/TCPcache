LD_LIBRARY_PATH=/home/opc/Kombain/libmodbus-2.9.2
export LD_LIBRARY_PATH
echo  $LD_LIBRARY_PATH

echo 1
echo $1
echo 2
echo $2
echo 3 
echo $3
#/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc  -o $2 $1

#/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc --help
#arm-linux-gnueabi-gcc main.c $(pkg-config --libs --cflags ~/BBB/build/lib/pkgconfig/libmodbus.pc)
/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc $1 -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -pthread $(pkg-config --libs --cflags /home/opc/Kombain/libmodbus-3.0.6/libmodbus.pc) -o $2