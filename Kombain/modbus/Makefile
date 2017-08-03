#
# Solid Makefile
#
CC = gcc
CFLAGS = -c -Wall 
LDFLAGS = -rdynamic -lrt -lm -ldl -lpthread
INC = -I/usr/include/modbus

# List of sources:
SOURCES = /home/opc/Kombain/modbus/test-client.c
OBJECTS = $(SOURCES:.c=.o)

# Name of executable target:
EXECUTABLE = test_mbus

    CFLAGS += `pkg-config --cflags libmodbus`
    #CFLAGS += `pkg-config --cflags gtk+-3.0`
    LDFLAGS += `pkg-config --libs libmodbus`
    #LDFLAGS += `pkg-config --libs gtk+-3.0`

all:   
	$(SOURCES) $(EXECUTABLE)

	$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(INC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
