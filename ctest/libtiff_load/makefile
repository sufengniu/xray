CC=gcc
CFLAGS=-g

all: scanline scanstrip

scanline: scanline.o
	$(CC) $(CFLAGS) scanline.o -o scanline -ltiff 

scanline.o: scanline.c
	$(CC) $(CFLAGS) -c scanline.c -o scanline.o

scanstrip: scanstrip.o
	$(CC) $(CFLAGS) scanstrip.o -o scanstrip -ltiff

scanstrip.o: scanstrip.c
	$(CC) $(CFLAGS) -c scanstrip.c -o scanstrip.o


tar: 
	tar cxvf *.c *.h makefile

clean:
	rm -rf *.o scanline scanstrip scantile output.tif *.dat
