CC=gcc
CFLAGS=-g

all:udp_client udp_server

udp_client: udp_client.o tif.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -ltiff -lrt

udp_client.o: udp_client.c udp_client.h sys_config.h tif.h
	$(CC) $(CFLAGS) -c udp_client.c -o udp_client.o

tif.o: tif.c tif.h sys_config.h
	$(CC) $(CFLAGS) -c tif.c -o tif.o

udp_server: udp_server.o tif.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -ltiff -lrt

udp_server.o: udp_server.c udp_server.h sys_config.h tif.h
	$(CC) $(CFLAGS) -c udp_server.c -o udp_server.o 

clean:
	rm -rf *.o udp_client udp_server
