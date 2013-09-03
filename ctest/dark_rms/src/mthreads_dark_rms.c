/*------------------------------------------------------
 * FILE: mthreads_dark_avg.c
 * DESCRIPTION: 
 *  
 * 
 * AUTHOR: Sufeng Niu 
 * LAST REVISED: 
 *------------------------------------------------------*/
#include "../include/sys_config.h"
#include "../include/tif.h"	// tif loading thread
#include "../include/rms.h"	// dark average operation
#include "../lib/thr_pool.h"
#include "../include/platform_init.h"

/* add customized thread header file here */

int main(int argc, char *argv[])
{

/*
 * initiliazation, threads number definition, parameters definition
 */
	void *status;
	int i;
	thr_pool_t *pool;
	pool = (thr_pool_t *)malloc(sizeof(thr_pool_t));
		
	NUM_THREADS = DEFAULT_THREADS_NUM;	// defined in sys_config.h	
	NUM_PROCESS = NUM_BLADES;
	
	if (argc == 3){
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

	/*------------------ tif loading ---------------------*/
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tif_start);
	
	tif_load(argv+2);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tif_stop);

	tif_accum = (tif_stop.tv_sec-tif_start.tv_sec)+(double)(tif_stop.tv_nsec-tif_start.tv_nsec)/(double)BILLION;
        printf("master thread: done in %lf second\n", tif_accum);
	
	/*------------------ processing threads ---------------------*/
	mem_alloc();
	buffer_size = buffer_length * buffer_width;
	printf("master thread: buffer size is %d\n", buffer_size);
	printf("master thread: buffer length is %d, buffer width is %d\n", buffer_length, buffer_width);

	/* assign thread number from 1 to NUM_PROCESS_THREADS */
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

	/*------------------ tif -----------------------*/
	printf("master thread: synthesis multiple chunks \n");
	printf("master thread: writing processed image to %s file \n", output_filename[1]);
	tif_syn();
	
	pthread_attr_destroy(&attr);
	thr_pool_destroy(pool);
	mem_free();
		
	return 0;
}
