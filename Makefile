CC = gcc
CFLAGS = -std=gnu99 -Wall -pedantic

all: i2cio
remap-test: i2cio.c
	$(CC) $(CFLAGS) -DREMAP_BIT_TEST i2cio.c -o $@

i2cio: i2cio.o main.o
main.o: main.c
i2cio.o: i2cio.c

clean:
	$(RM) *.o
	$(RM) i2cio

.PHONY: all clean
