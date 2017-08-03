rm *.o

gcc -Wall -g -c signals.c
#gcc -Wall -g  -c cashserver.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o server.o

gcc -Wall -g  client_pcsens.c -o client_pcsens
#gcc -Wall -g  pc_sens.c -o pc_sens


#gcc -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread  -o server server.o signals.o

#gcc -Wall -g -c client.c -D_REENTERANT -I/usr/include/nptl -L/usr/lib/nptl -lpthread -o client.o
#gcc -o client client.o signals.o
./client_pcsens
