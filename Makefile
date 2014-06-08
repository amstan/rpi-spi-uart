CC=gcc
CFLAGS=-c -g -Wall --std=c99
LDFLAGS=-lbcm2835
SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=spi-uart

all: $(SOURCES) $(EXECUTABLE)

run: all
	sudo ./$(EXECUTABLE)

clean:
	rm -f ${EXECUTABLE} ${OBJECTS}

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $< -o $@ $(CFLAGS)
