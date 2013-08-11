#ifndef IMAGE_OP_H_
#define IMAGE_OP_H_

#include "sys_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#include "mpi.h"

/* clock_t start, end */
struct timespec *start, *stop;
double accum;

void image_rms(int rank, MPI_Win *win, uint16 *strip_buff, MPI_Comm MPI_COMP_WORLD);

int rms_mem_alloc();
int rms_init();
int rms_clean();
void rms(int rank, MPI_Win *win, uint16 *strip_buff);
int rms_syn(int rank, MPI_Comm MPI_COMP_WORLD);

uint16 *dk0;
int16 *avg_buff;	// must be int16!
uint16 *rms_buff;
uint16 *output_image_avg;
uint16 *output_image_std;

#endif /* image_op.h */
