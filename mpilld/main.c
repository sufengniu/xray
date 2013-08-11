#include "sys_config.h"
#include "dark_mode.h"
#include "data_mode.h"

int main(int argc, char *argv[])
{

/*	
	char mode;
	if (argc == 3){
		mode = atoi(argv[2]);	// 1: dark mode, 2: data mode	
	}
	else{
		printf("Usage: exe m\n");
		printf("exe: executable file\n");
		printf("m: mode");
		exit(1);
	}
	
	if (mode == 1){
		printf("dark mode is selected\n");
*///		dkm(argc, argv);
/*		// dkm(image_info);
	}
	else if (mode == 2){
		printf("data mode is selected\n");
*/		dtm(argc, argv);
		// dtm(image_info);
/*	}
	else{
		printf("Error: wrong mode selection, must be 1 (dark mode) or 2 (data mode)!\n");
		exit(1);
	}
*/
	return 0;
}

/*
 * hardware information check
 * if GPU available, GPU check should be included
 * the hw check should include how many threads, stream processor, etc. 
 * the results use global variables, defined in sys_config.h file
 */
void hw_info()
{
	/* CPU processor check here */
	system("lscpu");

	system("nproc > procs_num");

	FILE *cpu_info;
	cpu_info = fopen("procs_num", "r");
	fscanf(cpu_info, "%d", &ttlprocs);

	fclose(cpu_info);

	/* GPU info checked here */

	/* other platform */
}


















