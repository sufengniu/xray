#ifndef AVG_H_
#define AVG_H_

#include <math.h>
#include "sys_config.h"

void *rms(void *arg);
int rms_init(int tid);
int rms_op(int tid);	// rms operation
int image_syn(int tid);

#endif /* avg.h */

