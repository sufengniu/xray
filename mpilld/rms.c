#include "rms.h"

#include "tif.h"
/*
 * rms perform average operation & stand derivation.
 * 
 *
 *
 */

void image_rms(int rank, MPI_Win *win, uint16 *strip_buff, MPI_Comm MPI_COMP_WORLD)
{

	/* create remote mem window */
	MPI_Win_create(MPI_BOTTOM, 0, sizeof(uint16), MPI_INFO_NULL, MPI_COMM_WORLD, win);

	/* allocate mem space */
	rms_mem_alloc();
	
	rms_init();
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	printf("%d process init done\n", rank);	
	
	rms(rank, win, strip_buff);	
	
	rms_syn(rank, MPI_COMP_WORLD);
	
	// rank 1 process perform tif writing
	if (rank == 1){
		tif_single_syn(image_info, output_image_avg, output_filename[0]);
		tif_single_syn(image_info, output_image_std, output_filename[1]);
	}	
	rms_clean();
	
}

int rms_mem_alloc()
{
	/* timer malloc */
	start = (struct timespec *)malloc(sizeof(struct timespec));
	stop = (struct timespec *)malloc(sizeof(struct timespec));

	/* user memory allocate here */
	//_________________________________________________________________

	if((dk0 = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for dk0!\n");
		exit(0);
	}

	if((avg_buff = (int16 *)malloc(image_info->buffer_size * sizeof(int16))) == NULL){
		printf("Could not allocate enough memory for avg_buff!\n");
		exit(0);
	}

	if((rms_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for rms_buff!\n");
		exit(0);
	}

	if((output_image_avg = (uint16 *)malloc(image_info->image_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for dark average output image!\n");
		exit(0);
	}
	if((output_image_std = (uint16 *)malloc(image_info->image_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for standard derivation output image!\n");
		exit(0);
	}

	//_________________________________________________________________

	return 0;
}

int rms_init()
{
	int i, j;	// i: column, j: row
	
	for (i = 0; i<image_info->buffer_length; i++){
		for (j = 0; j<image_info->buffer_width; j++){
			avg_buff[i * image_info->buffer_width + j] = 0;
			rms_buff[i * image_info->buffer_width + j] = 0;
		}
	}
	
	return 0;
}

int rms_clean()
{
	free(start);
	free(stop);	

	/* user memory free here */
	free(dk0);
	free(rms_buff);
	free(avg_buff);
	free(output_image_avg);
	free(output_image_std);
	
	return 0;
}

void rms(int rank, MPI_Win *win, uint16 *strip_buff)
{
	int i, j, k;	//i: column, j: row, k: page_num	
	long int image_index;
	int diff_sum;	
	int ping_pong = 0;
	
	printf("process %d: actived\n", rank);

	/* user function defined here */	
//__________________________________________________________________

	for (k = 0; k < image_info->page_num; k++) {
		
		MPI_Bcast(&ping_pong, 1, MPI_INT, 0, MPI_COMM_WORLD);		
	
		// RMA sync
		MPI_Win_fence(0, *win);
		// copy data from udp process to local
		int offset = (rank-1)*image_info->buffer_size + ping_pong*image_info->image_size;	
		MPI_Get(strip_buff, 
			image_info->buffer_size,
			MPI_INT16_T, 
			0, 
			offset,
			image_info->buffer_size, 
			MPI_INT16_T, 
			*win);	

		MPI_Win_fence(0, *win);	
	
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, start);
			
		if (k != 0){
			/* caculate in recursive method 3 */
			for (i = 0; i<image_info->buffer_length; i++){
				for (j = 0; j<image_info->buffer_width; j++){
					image_index = j + i * image_info->buffer_width;

					/* caculate in recursive method 3 */
					diff_sum = strip_buff[image_index] - dk0[image_index];

					/* rms */
					avg_buff[image_index] += diff_sum;
					rms_buff[image_index] += pow(diff_sum, 2);

				}
			}
		}
		else {
			for (i = 0; i<image_info->buffer_length; i++){
				for (j = 0; j<image_info->buffer_width; j++){
					image_index = j + i * image_info->buffer_width;

					dk0[image_index] = strip_buff[image_index];
				}
			}
		}	
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, stop);
		accum = (stop->tv_sec - start->tv_sec)+(double)(stop->tv_nsec - start->tv_nsec)/(double)BILLION; 
		printf("rank %d: rms computation done in %lf second\n", rank, accum);

	}
	
	printf("rank %d: dark image collection done\n", rank);
}

//__________________________________________________________________

int rms_syn(int rank, MPI_Comm MPI_COMP_WORLD){
	int i, j;
	long int image_index = 0;
	int avg_diff_sq;
	int avg_diff;
	int rms_sq;
	int temp_value;

	for (i = 0; i < image_info->buffer_length; i++){
		for (j = 0; j < image_info->buffer_width; j++){
			image_index = j + i * image_info->buffer_width + (rank-1) * image_info->buffer_size;

			/* caculate in recursive method 3 */
			// DKstd = sqrt(A2-(A1^2/numDark)/(numDark-1))
			avg_diff_sq = pow(avg_buff[j+i*image_info->buffer_width], 2);    // overflow may happen
			temp_value = avg_diff_sq / image_info->page_num;
			rms_sq = (rms_buff[j+i*image_info->buffer_width] - temp_value) / (image_info->page_num - 1);
			rms_buff[j+i*image_info->buffer_width] = sqrt(rms_sq);

			// DKavg = A1/numDark + dk0
			avg_diff = avg_buff[j+i*image_info->buffer_width] / image_info->page_num;
			avg_buff[j + i*image_info->buffer_width] = avg_diff + dk0[j + i*image_info->buffer_width];
						
		}
	}
	printf("rank %d: rms synthesis done\n", rank);
	
	MPI_Allgather(  rms_buff,
			image_info->buffer_size,
			MPI_INT16_T,
			output_image_std,
			image_info->buffer_size, 
			MPI_INT16_T,  
			MPI_COMP_WORLD);

	MPI_Allgather(  avg_buff, 
			image_info->buffer_size, 
			MPI_INT16_T, 
			output_image_avg,
			image_info->buffer_size, 
			MPI_INT16_T, 
			MPI_COMP_WORLD);
	
	return 0;
}
