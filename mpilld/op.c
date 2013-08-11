#include "op.h"

/*
 *
 *
 *
 *
 */

void *image_op(int rank, MPI_Win& win)
{

	/* allocate mem space*/
	op_mem_alloc(rank);	
	
	/* 2nd barrier, sync with udp_server.c init */
	MPI_Barrier(MPI_COMM_WORLD);
	/* create remote mem window */
	MPI_Win_create(MPI_BOTTOM, 0, sizeof(uint16), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	
	op
	
	op_clean();



	return (void *);
}

int op_mem_alloc(int rank)
{
	/* timer malloc */
	start = (struct timespec *)malloc(numthrds * sizeof(struct timespec));
	stop = (struct timespec *)malloc(numthrds * sizeof(struct timespec));
	accum = (double *)malloc(numthrds * sizeof(double));

	/* user memory allocate here */

	return 0;
}

int op_clean()
{
	free(start);
	free(stop);
	free(accum);

	/* user memory free here */

}

void op(void *sarg)
{
	int tid, pid;
	struct slave_arg *p = (struct slave_arg *)sarg;
	
	tid = p->tid;
	pid = p->pid;
	printf("process %d thread %d: actived\n", pid, tid);
	
	/* user function defined here */	
	
}
