
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int numthrds;

int main(int argc, char *argv[])
{
	printf("cpu infomation:\n");
	system("lscpu");

	system("nproc > thrs_num");

	FILE *cpu_info;
	cpu_info = fopen("thrs_num", "r");
	fscanf(cpu_info, "%d", &numthrds);

	printf("Threads in node: %d\n", numthrds);

	fclose(cpu_info);
	return 0;	
}
































































































