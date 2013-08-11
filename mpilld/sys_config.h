#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <tiffio.h>
#include <time.h>
#include <errno.h>
#include <math.h>

#define BILLION 1000000000L;

#define MAX_TIF			1000	// max tif file number

#define MAX_BLAEDS_NUM		2	// maxium number of blades
#define MAX_THREAD              100	// maxium threads number, defined by the hardware
#define BIT_PER_SAMPLE          16	// defined by camera property

#define alpha			3	// between [0, 4]
#define beta			15	// between [0, 20]

#define PORT			2346
static char *ipaddr = {"127.0.0.1"};

/* MPI global variables */
//___________________________________________________________

static int CPU_THRS_NUM;

int ttlprocs;				// total physical process number
int compprocs;				// computation proces in total
int numprocs;				// process number
int numthrds;				// threads number in each process

uint16 *strip_buff;     // parallel process image buffer
uint16 *image_buff;	// input image, remote mem acces enterance

typedef struct image_info_type{
	int width;
	int length;
	int page_num;
	int buffer_width;
	int buffer_length;
	int buffer_size;
	long int image_size;

} image_info_type;
struct image_info_type *image_info;

//____________________________________________________________

static char *output_filename[] = {"dark_avg.tif", "dark_rms.tif", "data.tif"};

struct timespec tif_start, tif_stop;
double tif_accum;

void hw_info();

void debug();

#endif
