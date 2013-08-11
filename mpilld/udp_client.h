#ifndef UDP_CLIENT_H_
#define UDP_CLIENT_H_

#include "sys_config.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>	// prerequisite typedef
#include <sys/socket.h> // struct sockaddr; system prototypes and constants
#include <netdb.h>    // network info lookup prototypes and structures
#include <netinet/in.h> // struct sockaddr_in; byte ordering macros
#include <arpa/inet.h>  // utility function prototypes

#include <math.h>

uint16 *sim_image;	// pointer for multiple images
uint16 *image_ptr;	// point for each frame

#define PACKET_SIZE	1024

typedef struct id_type {
	int bid;	// blade id
	// slave_thread_arg starg; // including pid and tid
} id_type;

void mem_alloc();
void mem_free();

#endif /* udp_client.h */
