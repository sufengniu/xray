/*------------------------------------------------------
 * FILE: mthreads_dark_avg.c
 * DESCRIPTION: 
 *  
 * 
 * AUTHOR: Sufeng Niu 
 * LAST REVISED: 
 *------------------------------------------------------*/
#include "../include/sys_config.h"
#include "../lib/thr_pool.h"
#include "../include/platform_init.h"

/* add customized thread header file here */
#include "../include/sub.h"
#include "../include/tif.h"     // tif loading thread
#include "../include/rms.h"     // dark average operation

int main(int argc, char *argv[])
{

/*
 * initiliazation, threads number definition, parameters definition
 */
	int i;
	thr_pool_t *pool;
	pool = (thr_pool_t *)malloc(sizeof(thr_pool_t));
	hit = 0;
			
	NUM_THREADS = DEFAULT_THREADS_NUM;	// defined in sys_config.h	
	NUM_PROCESS = NUM_BLADES;
	
	if (argc == 4){
		NUM_THREADS = atoi(argv[1]);
	} else{
		printf("Usage: exe n tif\n");
		printf("exe: executable file\n");
		printf("n: threads number\n");
		printf("tif: tif image file\n");
		exit(1);
	}
	
	if ((NUM_THREADS < 1)||(NUM_THREADS > MAX_THREAD)){
		printf("Error: the number of thread should between 1 and %d.\n", MAX_THREAD);
		exit(1);
	}
	
	NUM_PROCESS_THREADS = NUM_THREADS;

	hw_info();	// defined in platform_init.c

	/* pthread pool initial */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if ((pool = thr_pool_create(1, MAX_THREAD, 2, &attr)) == NULL){
		printf("Error: create thread pool failed!\n");
		exit(1);
	}

/*
 * tif.c API 
 * tif_infoi:	read the tif file to obtain image parameters. i.e: image size
 * tif_load:	load the tif file into the memory space
 * tif_syn:	tif file extracted out from memory,
 * 		stored as dark_avg.tif, dark_rms.tif, data.tif
 * tif_release:	free tif file memory
 */

	/*------------------ dark mode operation threads ---------------------*/
	/* tif loading */
	tif_info(argv+2);
	dk_mem_alloc();		// dark image mem allocation	
	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tif_start);	
	tif_load(input_image);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tif_stop);
	tif_accum = (tif_stop.tv_sec-tif_start.tv_sec)+(double)(tif_stop.tv_nsec-tif_start.tv_nsec)/(double)BILLION;
        printf("master thread: dark image loading done in %lf second\n", tif_accum);
	
	/* assign threads based on thread pool */
	for (i = 0; i<NUM_PROCESS_THREADS; i++){
		starg[i].tid = i;
		starg[i].pid = 0;

		thread_status = thr_pool_queue(pool, rms, (void *)(starg+i));
		if (thread_status == -1){
			printf("An error had occurred while adding a task!\n");
			exit(0);
		}
	}
	/* wait for all threads complete */	
	thr_pool_wait(pool);
	
	printf("master thread: dark mode is done!\n");
	printf("---------------------------------------------\n");
	printf("master thread: starting data mode...\n");

/*
 *
 *
 */
	
	/*------------------ data mode operation threads ---------------------*/
	/* loading data image */
	tif_info(argv+3);
	dt_mem_alloc();		// data image mem allocation

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tif_start);
	tif_load(data_image);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tif_stop);
	tif_accum = (tif_stop.tv_sec-tif_start.tv_sec)+(double)(tif_stop.tv_nsec-tif_start.tv_nsec)/(double)BILLION;
	printf("master thread: data image loading done in %lf second\n", tif_accum);
	/* assign thread number from 1 to NUM_PROCESS_THREADS */
	for (i = 0; i<NUM_PROCESS_THREADS; i++){
		starg[i].tid = i;
		starg[i].pid = 0;

		thread_status = thr_pool_queue(pool, sub, (void *)(starg+i));
		if (thread_status == -1){
			printf("An error had occurred while adding a task!\n");
			exit(0);
		}
	}
	thr_pool_wait(pool);

	/*------------------ tif write --------------------*/
	printf("master thread: synthesis multiple chunks \n");
	printf("master thread: writing average image to %s file \n", output_filename[0]);
	printf("master thread: writing rms image to %s file \n", output_filename[1]);

	tif_syn();
	
	/* clean up */
	pthread_attr_destroy(&attr);
	thr_pool_destroy(pool);
	dk_mem_free();          // free dark image mem
	dt_mem_free();
		
	return 0;
}
