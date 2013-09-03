

CC=gcc
CFLAGS=-g -Wall

all: cpu_info
	
cpu_info: cpu_info.o
	$(CC) $(CFLAGS) $^ -o $@

cpu_info.o: cpu_info.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -rf *.o cpu_info
	


