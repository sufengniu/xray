#ifndef IMAGE_OP_H_
#define IMAGE_OP_H_

#include "sys_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "mpi.h"

/* clock_t start, end */
struct timespec *start, *stop;
double accum;

/* mask to remove bad strip, you can define more bad strip as XXX_N format */
int *x_low_bound, *x_high_bound;
int *y_low_bound, *y_high_bound;

#define BAD_STRIP               1
#define XLB_1                   109     // x low bound
#define XHB_1                   120     // x high bound
#define YLB_1                   0       // y low bound
#define YHB_1                   961     // y high bound

void image_sub(int rank, MPI_Win *win, uint16 *strip_buff, MPI_Comm MPI_FOR_WORLD, MPI_Comm MPI_BACK_WORLD, MPI_Comm MPI_COMP_WORLD);

char buf[128];

int sub_mem_alloc();
int sub_clean();
int sub_init(int subrank, MPI_Comm MPI_COMP_WORLD);
void sub(int subrank, MPI_Win *win, uint16 *strip_buff, MPI_Comm MPI_FOR_WORLD, MPI_Comm MPI_BACK_WORLD);
int data_write(int rank, MPI_Comm MPI_BACK_WORLD);

uint16 *avg_buff;	// must be uint16
uint16 *rms_buff;
uint16 *mask_buff;
uint16 *res_buff;

uint16 *data_image;
uint16 *avg_image;
uint16 *rms_image;

#endif /* image_sub.h */
