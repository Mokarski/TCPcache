#ln -s /opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc  /usr/bin/arm-linux-gnueabi-gcc

#ln -s /usr/bin/arm-linux-gnueabi-gcc-4.5 /usr/bin/arm-linux-gnueabi-gcc

#CSTOOL_DIR=/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin

PATH=$PATH:/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin
export PATH
echo $PATH

export CC=arm-fsl-linux-gnueabi-gcc
export CXX=arm-fsl-linux-gnueabi-g++
echo $CC
echo $CXX
#./configure --host=arm-linux-gnueabi --build=x86_64-linux-gnu --prefix=/home/opc/Kombain/test
#./configure --host=arm-linux-gnueabi --build=x86_64-linux-gnu --prefix=${CSTOOL_DIR}/linux_arm_tool



./configure --host=arm-linux-gnueabi CC=arm-linux-gnueabi-gcc \
    AR=arm-linux-gnueabi-ar STRIP=arm-linux-gnueabi-strip RANLIB=arm-linux-gnueabi-ranlib \
       CPPFLAGS="-mfpu=neon -I/path/to/zlib/include/files" LDFLAGS="-L/path/to/zlib/lib/files"  \
         --prefix=/home/opc/Kombain/test
#          --prefix=/path/to/dir/for/output/files   



echo $PATH
sudo make
echo $PATH
sudo make install
