CC=gcc
CFLAGS=-g -Wall

all: multi_scan test

multi_scan: multi_scan.o
	$(CC) $(CFLAGS) multi_scan.o -o multi_scan -ltiff -lrt 

multi_scan.o: multi_scan.c
	$(CC) $(CFLAGS) -c multi_scan.c -o multi_scan.o

test: test.o
	$(CC) $(CFLAGS) test.o -o test -ltiff

test.o: test.c
	$(CC) $(CFLAGS) -c test.c -o test.o

tar: 
	tar cxvf *.c *.h makefile

clean:
	rm -rf *.o multi_scan test *.dat
