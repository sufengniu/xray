#include "sub.h"

#include "tif.h"

/*
 * communicator explaination:
 * Assuming we have 8 processes;
 * MPI_FOR_WORLD: process 0, 2 to process 7
 * MPI_COMP_WORLD: process 3 to process 7
 * MPI_BACK_WORLD: process 1 to process 7
 * process 0: data acquisition
 * process 1: tif data writing
 *
 */

void image_sub(	int rank, 
		MPI_Win *win, 
		uint16 *strip_buff, 
		MPI_Comm MPI_FOR_WORLD, 
		MPI_Comm MPI_BACK_WORLD,
		MPI_Comm MPI_COMP_WORLD)
{
	int subrank;
	MPI_Comm_rank(MPI_COMP_WORLD, &subrank);
	
	/* create remote mem window */
        MPI_Win_create(MPI_BOTTOM, 0, sizeof(uint16), MPI_INFO_NULL, MPI_FOR_WORLD, win);
	
	/* allocate mem space*/
	sub_mem_alloc();	
	sub_init(subrank, MPI_COMP_WORLD);
	
	MPI_Barrier(MPI_COMM_WORLD);

	printf("%d process init done\n", rank);
	
	sub(rank, win, strip_buff, MPI_FOR_WORLD, MPI_BACK_WORLD);
	
	sub_clean();
		
	/* MPI parallel IO can be considered in the future */
	
}

int sub_mem_alloc()
{
	/* timer malloc */
	start = (struct timespec *)malloc(sizeof(struct timespec));
	stop = (struct timespec *)malloc(sizeof(struct timespec));

	/* user memory allocate here */
	x_low_bound = (int *)malloc(BAD_STRIP * sizeof(int));
	y_low_bound = (int *)malloc(BAD_STRIP * sizeof(int));
	x_high_bound = (int *)malloc(BAD_STRIP * sizeof(int));
	y_high_bound = (int *)malloc(BAD_STRIP * sizeof(int));

	if((mask_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for mask buffer!\n");
		exit(0);
	}	

	if((res_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for mask buffer!\n");
		exit(0);
	}

	if((avg_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for avg_buff!\n");
		exit(0);
	}

	if((rms_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for rms_buff!\n");
		exit(0);
	}
	
	if((data_image = (uint16 *)malloc(image_info->image_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for rms_buff!\n");
		exit(0);
	}
	
	return 0;
}

int sub_init(int subrank, MPI_Comm MPI_COMP_WORLD)
{
	int i, j;
		
	for (i = 0; i<image_info->buffer_length; i++){
		for (j = 0; j<image_info->buffer_width; j++){
			res_buff[j+i*image_info->buffer_width] = 0;
		}		
	}
	
	if (subrank == 0){
		if ((avg_image = (uint16 *)malloc(image_info->image_size * sizeof(uint16))) == NULL){
			printf("Could not allocate enough memory for avg!\n");
			exit(0);
		}
		if ((rms_image = (uint16 *)malloc(image_info->image_size * sizeof(uint16))) == NULL){
			printf("Could not allocate enough memory for rms!\n");
			exit(0);
		}
		
		tif_info(output_filename);
		tif_single_load(avg_image);
		
		tif_info(output_filename+1);
		tif_single_load(rms_image);
		
		MPI_Scatter(avg_image, image_info->buffer_size, MPI_INT16_T, avg_buff, image_info->buffer_size, MPI_INT16_T, 0, MPI_COMP_WORLD);
		MPI_Scatter(rms_image, image_info->buffer_size, MPI_INT16_T, rms_buff, image_info->buffer_size, MPI_INT16_T, 0, MPI_COMP_WORLD);
		
		free(avg_image);
		free(rms_image);	
	}
	else {

		MPI_Scatter(avg_image, image_info->buffer_size, MPI_INT16_T, avg_buff, image_info->buffer_size, MPI_INT16_T, 0, MPI_COMP_WORLD);
		MPI_Scatter(rms_image, image_info->buffer_size, MPI_INT16_T, rms_buff, image_info->buffer_size, MPI_INT16_T, 0, MPI_COMP_WORLD);

	}
	
	/* image bad strip
	 * x: 109 ~ 120
	 * y: 0 ~ 961
	 */
	*x_low_bound = XLB_1;
	*x_high_bound = XHB_1;
	*y_low_bound = YLB_1;
	*y_high_bound = YHB_1;
	
	/* add bad strip boundary condition here */

	
	// bad strip boundary condition	
	for (i = 0; i<image_info->buffer_length; i++){
		for (j = 0; j<image_info->buffer_width; j++){
			
			if ((subrank*image_info->buffer_length >= *y_low_bound) || ((subrank+1)*image_info->buffer_length <= *y_high_bound)) {
				if ((j>=*x_low_bound) && (j<=*x_high_bound)){
					if ((i<=(*y_high_bound - subrank*image_info->buffer_length)) || (i>=(subrank*image_info->buffer_length - *y_low_bound)))
						mask_buff[i * image_info->buffer_width + j] = 0;
					else
						mask_buff[i * image_info->buffer_width + j] = 1;
				}
				else
					mask_buff[i * image_info->buffer_width + j] = 1;
			}
			else {
				mask_buff[i * image_info->buffer_width + j] = 1;
			}
		}
	}

	return 0;
}

int sub_clean()
{
	free(start);
	free(stop);	

	/* user memory free here */
	free(mask_buff);
	free(res_buff);
	free(avg_buff);
	free(rms_buff);
	free(data_image);
}

void sub(int rank, MPI_Win *win, uint16 *strip_buff, MPI_Comm MPI_FOR_WORLD, MPI_Comm MPI_BACK_WORLD)
{
	int i, j, k;	// i: column, j: row, k: page_num
	long int image_index = 0;
	int lld;
	int res_pixel;
	int ping_pong = 0;

	printf("process %d: actived\n", rank);

	/* user function defined here */	
//_______________________________________________________________

	for (k = 0; k < image_info->page_num; k++) {
			
		MPI_Bcast(&ping_pong, 1, MPI_INT, 0, MPI_FOR_WORLD);
	
		// RMA sync
		MPI_Win_fence(0, *win);
		// copy data from udp process to local
		int offset = (rank-2)*image_info->buffer_size + ping_pong*image_info->image_size;
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

		for (i = 0; i<image_info->buffer_length; i++){
			for (j = 0; j<image_info->buffer_width; j++){
				image_index = j + i * image_info->buffer_width;
				
				lld = alpha * rms_buff[image_index] + beta;
			
				res_pixel = strip_buff[image_index] - avg_buff[image_index];
					
				if (res_pixel >= lld){
					res_buff[image_index] = res_pixel;		
				}
				else {
					res_buff[image_index] = 0;
				}
				res_buff[image_index] = res_buff[image_index] * mask_buff[image_index];
		
			}
		}
		
		/* parallel tif IO should be considered for acceleration */

		MPI_Gather(     res_buff,
				image_info->buffer_size,
				MPI_INT16_T,
				data_image,
				image_info->buffer_size,
				MPI_INT16_T,
				0,
				MPI_BACK_WORLD);
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, stop);
		accum = (stop->tv_sec - start->tv_sec)+(double)(stop->tv_nsec - start->tv_nsec)/(double)BILLION;

		printf("rank %d: sub computation done in %lf second\n", rank, accum);
		
	}

	printf("rank %d: data image subtraction done\n", rank);
}

// data write process
int data_write(int rank, MPI_Comm MPI_BACK_WORLD)
{
	int k;

	if((res_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for res_buff!\n");
		exit(0);
	}

//	if((data_image = (uint16 *)malloc(image_info->image_size * sizeof(uint16))) == NULL){
	if((data_image = (uint16 *)malloc(image_info->buffer_size * (compprocs+1) * sizeof(uint16))) == NULL){
		printf("Could not allocate enough memory for data_image\n");
		exit(0);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	for(k = 0; k < image_info->page_num; k++){

		// gather, then serial tif write
		MPI_Gather(     res_buff,
				image_info->buffer_size,
				MPI_INT16_T,
				data_image,
				image_info->buffer_size,
				MPI_INT16_T,
				0,
				MPI_BACK_WORLD);

		strcpy(buf, "data");
		sprintf(buf+4, "%03d", k);
		strcat(buf, ".tif");
		tif_single_syn(image_info, data_image, buf);	
	}
	
	free(res_buff);
	free(data_image);
	return 0;
}
//__________________________________________________________________

 
