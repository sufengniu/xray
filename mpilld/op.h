#ifndef IMAGE_OP_H_
#define IMAGE_OP_H_

#include "sys_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "thr_pool.h"
#include "mpi.h"

/* clock_t start, end */
struct timespec *start, *stop;
double *accum;

void *image_op(int argc, char *argv[], uint16 *image);

int op_mem_alloc();
int op_clean();
void *op(void *sarg);

#endif /* image_op.h */
