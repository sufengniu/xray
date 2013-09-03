#ifndef PLATFORM_INIT_H_
#define PLATFORM_INIT_H_

#include <stdlib.h>
#include <stdio.h>
#include "sys_config.h"

void hw_info();
void dk_mem_alloc();	// dark image
void dt_mem_alloc();	// data image
void dk_mem_free();
void dt_mem_free();

#endif /* platform_init.h */
