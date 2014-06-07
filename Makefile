CC=gcc
CFLAGS=-c -g -Wall -I/usr/src/linux/include
LDFLAGS=
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=spi-uart

all: $(SOURCES) $(EXECUTABLE)

run: all
	sudo ./$(EXECUTABLE) -D /dev/spidev0.0 -b 9 -s 243400

clean:
	rm -f ${EXECUTABLE} ${OBJECTS}

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $< -o $@ $(CFLAGS)
