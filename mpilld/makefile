CC=mpicc
CFLAGS=-O3 

all: main udp_client

main: main.o dark_mode.o rms.o udp_server.o tif.o data_mode.o sub.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -ltiff -lrt

main.o: main.c dark_mode.h data_mode.h rms.h udp_server.h sys_config.h
	$(CC) $(CFLAGS) -c $< -o $@

udp_server.o: udp_server.c udp_server.h sys_config.h
	$(CC) $(CFLAGS) -c $< -o $@

dark_mode.o: dark_mode.c dark_mode.h udp_server.h tif.h rms.h sys_config.h
	$(CC) $(CFLAGS) -c $< -o $@

rms.o: rms.c rms.h sys_config.h tif.h
	$(CC) $(CFLAGS) -c $< -o $@

data_mode.o: data_mode.c data_mode.h udp_server.h tif.h sub.h sys_config.h
	$(CC) $(CFLAGS) -c $< -o $@

sub.o: sub.c sub.h sys_config.h tif.h
	$(CC) $(CFLAGS) -c $< -o $@

udp_client: udp_client.o tif.o
	gcc $(CFLAGS) $^ -o $@ -lm -ltiff -lrt

udp_client.o: udp_client.c udp_client.h sys_config.h tif.h
	gcc $(CFLAGS) -c $< -o $@

tif.o: tif.c tif.h sys_config.h
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o udp_client main data*.tif 
