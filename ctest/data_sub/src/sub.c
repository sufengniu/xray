#include "../include/sub.h"

/*
 * DESCRIPTION: image subtraction to compensate background noise
 * dark average and dark root mean square op are needed
 *
 * USAGE: just call sub function name by threadpool, you can use
 * ex: thr_pool_queue(pool, sub, (void *)(starg+i))
 * where starg is a pointer to define thread id and process id
 * mask also avaliable 
 */

void *sub(void *arg)
{
	int tid;
	slave_thread_arg *p = (slave_thread_arg *)arg;

	tid = p->tid;
	printf("thread %d: actived\n", tid);
	mask_init(tid);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, start + tid);
	sub_op(tid);
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, stop + tid);
	accum[tid] = ((stop+tid)->tv_sec - (start+tid)->tv_sec)+(double)((stop+tid)->tv_nsec - (start+tid)->tv_nsec)/(double)BILLION;
	
	printf("thread %d: data subtraction done in %lf second\n", tid, accum[tid]);
	
}

int mask_init(tid){
	int i, j;       // i: column, j: row

	/* image bad strip
	 * x: 109 ~ 120
	 * y: 0 ~ 961
	 */
	*x_low_bound = 109;
	*x_high_bound = 120;
	*y_low_bound = 0;
	*y_high_bound = 961;

	for (i = 0; i<buffer_length; i++){
		for (j = 0; j<buffer_width; j++){
			if ((j>=*x_low_bound) && (j<=*x_high_bound) && (i>=*y_low_bound) && (i<=*y_high_bound))
				*(*(mask_buffer + tid) + i * buffer_width + j) = 0;	
			else
				*(*(mask_buffer + tid) + i * buffer_width + j) = 1;
		}
	}

	return 0;
}

int sub_op(int tid)
{
	int i, j, k;	// i: column, j: row, k: page num
	long int image_index = 0;
	int lld;
	int res_pixel;
	printf("thread %d: data subtraction operation begin ... \n", tid);
	
	for (k = 0; k < pages; k++){

		for (i = 0; i<buffer_length; i++){
			for (j = 0; j<buffer_width; j++){
				image_index = j + i * buffer_width + tid * buffer_size + k * page_size;
				
				lld = alpha * rms_buffer[tid][j+i*buffer_width] + beta;
				res_pixel = data_image[image_index] - avg_buffer[tid][j+i*buffer_width];
			
				if (res_pixel >= lld){
					res_image[image_index] = res_pixel;
				}
				else {
					res_image[image_index] = 0;
				}

				res_image[image_index] = res_image[image_index] * mask_buffer[tid][j+i*buffer_width]; 
			}
		}
	}
	return 0;
}

