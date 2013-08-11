#include "sys_config.h"
#include "data_mode.h"
#include "mpi.h"

/* add customized header here */
#include "sub.h"
#include "udp_server.h"
#include "tif.h"

int dtm(int argc, char *argv[])
{	
	int rank;
	int i;

	int *comp_rank, *forward_rank, *backward_rank;

	/* MPI Initialization */
	MPI_Init (&argc, &argv);
	MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Status status;

	if (rank == 0){
		printf("---------------------------------\n");
		printf("---- data subtraction begins ----\n");
		printf("---------------------------------\n");
		printf("-- hardware information: \n");
		printf("-- cpu information: \n");
		fflush(stdout);	
		hw_info();
	}

	// public window for rma
	MPI_Win win;		
	// debug();

	if (rank == 0){
		printf("total physical process: %d\n", ttlprocs);
		fflush(stdout);
		if (ttlprocs > numprocs)
		{
			printf("Warning: number of process is not the same as cpu core number\n");
			printf("this will affect computation performance\n");
		}
	}

	// create a new group and MPI_Comm
	// ex: (N: not available)
	// MPI_COMM_WORLD rank: 0 1 2 3 4 5 6
	// MPI_FOR_WORLD rank:  0 N 1 2 3 4 5
	// MPI_BACK_WORLD rank: N 0 1 2 3 4 5
	// MPI_COMP_WORLD rank: N N 0 1 2 3 4
	forward_rank = (int *)malloc((numprocs-1) * sizeof(int));
	backward_rank = (int *)malloc((numprocs-1) * sizeof(int));
	comp_rank = (int *)malloc((numprocs-2) * sizeof(int));

	for (i=0; i<numprocs-1; i++){
		if (i == 0)
			forward_rank[i] = i;
		else
			forward_rank[i] = i+1;
	}	
	for (i=0; i<numprocs-1; i++){	
		backward_rank[i] = i+1;
	}	
	for (i=2; i<numprocs; i++){	
		comp_rank[i-2] = i;	
	}	
	
	MPI_Group global_group, fgroup, bgroup, cgroup;     // forward group, backward group, computation group
	MPI_Comm MPI_FOR_WORLD, MPI_BACK_WORLD, MPI_COMP_WORLD;        // forward, backward, computation communicator
	MPI_Comm_group(MPI_COMM_WORLD, &global_group);

	MPI_Group_incl(global_group, numprocs-1, backward_rank, &bgroup);
	MPI_Comm_create(MPI_COMM_WORLD, bgroup, &MPI_BACK_WORLD);

	MPI_Group_incl(global_group, numprocs-1, forward_rank, &fgroup);
	MPI_Comm_create(MPI_COMM_WORLD, fgroup, &MPI_FOR_WORLD);

	MPI_Group_incl(global_group, numprocs-2, comp_rank, &cgroup);	
	MPI_Comm_create(MPI_COMM_WORLD, cgroup, &MPI_COMP_WORLD);

	// build MPI struct for image_info
	int count = 7;
	MPI_Datatype MPI_Imageinfo;
	MPI_Datatype types[7] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_LONG};
	int len[7] = {1, 1, 1, 1, 1, 1, 1};
	MPI_Aint disp[7];
	long int base;
	MPI_Address(image_info, disp);
	MPI_Address(&(image_info->width), disp+1);
	MPI_Address(&(image_info->length), disp+2);
	MPI_Address(&(image_info->page_num), disp+3);
	MPI_Address(&(image_info->buffer_width), disp+4);
	MPI_Address(&(image_info->buffer_length), disp+5);
	MPI_Address(&(image_info->buffer_size), disp+6);
	MPI_Address(&(image_info->image_size), disp+7);
	base = disp[0];
	for(i=0; i<8; i++)
		disp[i] -= base;

	MPI_Type_struct(count, len, disp, types, &MPI_Imageinfo);
	MPI_Type_commit(&MPI_Imageinfo);

	image_info = (struct image_info_type *)malloc(sizeof(struct image_info_type));

	MPI_Barrier(MPI_COMM_WORLD);

	// check processor rank	
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(processor_name, &name_len);
	printf("-- processor %s, rank %d out of %d processors\n", processor_name, rank, numprocs);
	fflush(stdout);

	// udp need 1 process, and 1 process for writing to disk
	compprocs = numprocs - 2;

	// 1st barrier
	MPI_Barrier(MPI_COMM_WORLD);

	/* 
	 * master node is iocfccd1. in hostfile iocfccd1 should be placed as the 1st
	 * check iocfccd1's rank. it should be rank = 0
	 */
	if (rank == 0) {

		/*
		 * udp routine here
		 * the input image mem space is defined in sys_config.h
		 * input image stores in image_buff (global)
		 * image_buff mem allocated in udp 
		 */
		udp_setup();
	
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(image_info, 1, MPI_Imageinfo, 0, MPI_COMM_WORLD);
			
		udp_sub(&win, &MPI_FOR_WORLD);
	}
	else if (rank == 1){
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(image_info, 1, MPI_Imageinfo, 0, MPI_COMM_WORLD);
		
		// perform data write to a raid disk
		data_write(rank, MPI_BACK_WORLD);
	}
	else {
		// sync for image_info
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(image_info, 1, MPI_Imageinfo, 0, MPI_COMM_WORLD);

		// malloc strip_buff
		if((strip_buff = (uint16 *)malloc(image_info->buffer_size * sizeof(uint16))) == NULL){
			printf("Could not allocate enough memory for strip_buff!\n");
			exit(0);
		}

		/* sub operation */
		image_sub(rank, &win, strip_buff, MPI_FOR_WORLD, MPI_BACK_WORLD, MPI_COMP_WORLD);

		/* add your computation routine here */

	}

	MPI_Barrier(MPI_COMM_WORLD);
	
	/* clean up */
	free(strip_buff);
	free(comp_rank);
	free(forward_rank);
	free(backward_rank);	
	free(image_info);
	
	if ((rank != 0) && (rank != (numprocs-1))){
		MPI_Comm_free(&MPI_COMP_WORLD);
	}
	if (rank != (numprocs-1)){
		MPI_Comm_free(&MPI_FOR_WORLD);
	}
	if (rank != 0){
		MPI_Comm_free(&MPI_BACK_WORLD);
	}
	if (rank != (numprocs-1)){	
		MPI_Win_free(&win);
	}
	MPI_Finalize();

	return 0;
}

void debug()
{
	int i = 0;
	char hostname[MPI_MAX_PROCESSOR_NAME];

	gethostname(hostname, sizeof(hostname));
	printf("PID %d on %s ready for attach\n", getpid(), hostname);
	fflush(stdout);
	while(0 == i)
		sleep(5);
}

