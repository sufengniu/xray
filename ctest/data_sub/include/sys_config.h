#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <tiffio.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define BILLION 1000000000L;

#define NUM_CORE		8
#define NUM_BLADES		1

#define MAX_THREAD              8	// maxium threads number, defined by the hardware
#define BIT_PER_SAMPLE          16	// defined by camera property
#define DEFAULT_THREADS_NUM     8	// default threads number

#define alpha			3	// between [0, 4]
#define beta			15	// between [0, 20]

/* image tag, tif.c need to be modefied */
#define LENGTH                  2000
#define WIDTH                   1152

#define BAD_STRIP		1

int NUM_THREADS, NUM_PROCESS_THREADS;	// threads number, number of processing threads
int NUM_PROCESS;			// process number

/* dark image mem and buffer address */
uint16 *input_image, *output_image_avg, *output_image_std;
uint16 **dk0;		// first dark image for recursive computation
int16 **avg_buffer;	// set as int16 for avg method 3 computation
uint16 **rms_buffer;
uint16 **mask_buffer;	// mask image

/* data image mem and buffer address */
uint16 *data_image;
uint16 *res_image;
uint16 **data_buffer;

int *x_low_bound, *x_high_bound;
int *y_low_bound, *y_high_bound;

int buffer_width, buffer_length;
int buffer_size;
int pages, page_size;

static const char *output_filename[] = {"dark_avg.tif", "dark_rms.tif", "data.tif"};

static long int *hit;

int thread_status;	// threads status for hand shaking

/* clock_t start, end */
struct timespec *start, *stop;
double *accum;
struct timespec tif_start, tif_stop;
double tif_accum;

/* arg for slave threads */
typedef struct slave_thread_arg
{
	int tid;	// thread id
	int pid;	// process id
} slave_thread_arg;
struct slave_thread_arg *starg; // slave thread arguments

#endif
