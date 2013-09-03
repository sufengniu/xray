#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_

#include "sys_config.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  // prerequisite typedef
#include <sys/socket.h> // struct sockaddr; system prototypes and constants
#include <netdb.h>    // network info lookup prototypes and structures
#include <netinet/in.h> // struct sockaddr_in; byte ordering macros
#include <arpa/inet.h>  // utility function prototypes

#include <math.h>

#define PACKET_SIZE     1024

typedef struct id_type {
        int bid;        // blade id
        // slave_thread_arg starg; // including pid and tid
} id_type;

uint16 *image_buff;
uint16 *image_ptr;
uint16 *strip;
void mem_alloc();
void mem_free();

#endif /* udp_server.h */
