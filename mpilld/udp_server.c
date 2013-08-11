#include "udp_server.h"

#include "tif.h"

/*
 * udp contains following parts: udp socket setup; image parameter receive and image receive
 * image parameters need to be broadcast to other process for mem allocate and frame init
 * the image paramter is defined in sys_config.h as image_info
 * images receive page number should be included in image parameters 
 */

void udp_setup()
{

	packet_size = PACKET_SIZE;	
	
	/* setup udp socket */
	printf("#: UDP socket setup...\n");
	if((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("#: Error: socket init failed!\n");
		exit(0);
	}
	client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	
	client_addr->sin_family = AF_INET;
	client_addr->sin_port = htons(PORT);
	client_addr->sin_addr.s_addr = inet_addr(ipaddr);

	if(bind(sid, (struct sockaddr *)client_addr, sizeof(struct sockaddr)) == -1) {
		printf("#: Error: bind failed\n");
		exit(0);
	}
	size = sizeof(struct sockaddr_in);	
		
	/* hand shaking */	
	struct id_type sendid, recvid;
	sendid.bid = 1;	
	recvfrom(sid, &recvid, sizeof(struct id_type), MSG_WAITALL, (struct sockaddr *)client_addr, &size);
	printf("#: Receive hand shaking signal from Blade %d\n", recvid.bid);
	fflush(stdout);
	
	sendto(sid, &sendid, sizeof(struct id_type), 0, (struct sockaddr *)client_addr, size);

	//_________________________________________________________

	recvfrom(sid, image_info, sizeof(struct image_info_type), MSG_WAITALL, (struct sockaddr *)client_addr, &size);
	
	num_packets = ceil((float)image_info->image_size/packet_size);
	image_info->buffer_length = ceil((float)image_info->length/compprocs);
	image_info->buffer_width = image_info->width;
	image_info->buffer_size = image_info->buffer_length * image_info->buffer_width;
	image_info->image_size = image_info->buffer_size * compprocs;
	printf("#: image info:\n");
	printf("#: image length: %d, image width: %d\n", image_info->length, image_info->width);
	printf("#: computation process number: %d\n", compprocs);
	printf("#: image strip buffer width: %d, buffer length: %d\n", image_info->buffer_width, image_info->buffer_length);
	printf("#: strip buffer size: %d, image size: %d\n", image_info->buffer_size, image_info->image_size);
	printf("#: image page number: %d\n", image_info->page_num);
	printf("#: packet number per frame: %d\n", num_packets);

	mem_alloc();

	// image_ptr = image_buff;	

}

void udp_sub(MPI_Win *win, MPI_Comm *MPI_FOR_WORLD)
{
	int i, j, k;
	int ping_pong = 1;

	/* remote memory access */
	MPI_Win_create(image_buff, 2*image_info->image_size*sizeof(uint16), sizeof(uint16), MPI_INFO_NULL, *MPI_FOR_WORLD, win);

	MPI_Barrier(MPI_COMM_WORLD);

	//_________________________________________________________

	for (i = 0; i < image_info->page_num; i++)
	{
		ping_pong ^= 1; // toggle ping pong between 1 and 0

		// image_ptr = image_buff + ping_pong*image_info->image_size;

		for (j = 0; j < num_packets; j++)
		{

			// recvfrom(sid, image_ptr, 2*packet_size, 0, (struct sockaddr *)client_addr, &size);
			// image_ptr += packet_size;
			recvfrom(sid, strip, 2*packet_size, 0, (struct sockaddr *)client_addr, &size);
			if(j<num_packets-1){
				for (k = 0; k < packet_size; k++){
					image_buff[k+j*packet_size+ping_pong*image_info->image_size] = strip[k];
				}
			}
			else {
				int fi;         // final index in each frame
				fi = image_info->image_size - j*packet_size;
				for (k = 0; k < fi; k++){
					image_buff[k+j*packet_size+ping_pong*image_info->image_size] = strip[k];
				}
			}
		}

		printf("#: Receiving %dth image frame\n", i);

		MPI_Bcast(&ping_pong, 1, MPI_INT, 0, *MPI_FOR_WORLD);

		MPI_Win_fence(0, *win);

		MPI_Win_fence(0, *win);
	}
	/*
	   while()
	   {
	   ping_pong != ping_pong;
	   image_ptr = image_buff + ping_pong*image_info->image_size;

	   for (j = 0; j < num_packets; j++)
	   {

	   recvfrom(sid, image_ptr, packet_size, 0, (struct sockaddr *)client_addr, &size);       
	   image_ptr += packet_size;
	   }

	   printf("~ Receiving %dth image frame\n", i);
	   image_info->page_num++;
	   MPI_Barrier(MPI_COMM_WORLD);
	   }
	   */

	printf("#: Receiving done\n");
	close(sid);

	printf("#: -- finished --\n");

	free(client_addr);
	mem_free();


}

void udp_rms(MPI_Win *win)
{	
	int i, j, k;	
	int ping_pong = 1;
	
	/* remote memory access */
	MPI_Win_create(image_buff, 2*image_info->image_size*sizeof(uint16), sizeof(uint16), MPI_INFO_NULL, MPI_COMM_WORLD, win);
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	//_________________________________________________________

	for (i = 0; i < image_info->page_num; i++)
	{	
		ping_pong ^= 1;	// toggle ping pong between 1 and 0

		// image_ptr = image_buff + ping_pong*image_info->image_size;

		for (j = 0; j < num_packets; j++)
		{

			// recvfrom(sid, image_ptr, 2*packet_size, 0, (struct sockaddr *)client_addr, &size);
			// image_ptr += packet_size;

			recvfrom(sid, strip, 2*packet_size, 0, (struct sockaddr *)client_addr, &size);
			if(j<num_packets-1){	
				for (k = 0; k < packet_size; k++){
					image_buff[k+j*packet_size+ping_pong*image_info->image_size] = strip[k];
				}
			}
			else {
				int fi;		// final index in each frame
				fi = image_info->image_size - j*packet_size;
				for (k = 0; k < fi; k++){
					image_buff[k+j*packet_size+ping_pong*image_info->image_size] = strip[k];
				}
			}
		}

		printf("#: Receiving %dth image frame\n", i);
		
		MPI_Bcast(&ping_pong, 1, MPI_INT, 0, MPI_COMM_WORLD);

		MPI_Win_fence(0, *win);

		MPI_Win_fence(0, *win);
	}

	printf("#: Receiving done\n");
	close(sid);

	printf("#: -- finished --\n");
		
	free(client_addr);
	mem_free();
}

void mem_alloc()
{
	if((image_buff = (uint16 *)malloc(2 * image_info->image_size * sizeof(uint16))) == NULL)
	{
		printf("#: Error: could not allocate enought memory for image_buff!\n");
		exit(0);
	}	
	if((strip = (uint16 *)malloc(packet_size * sizeof(uint16))) == NULL)
	{
		printf("#: Error: could not allocate enought memory for strip!\n");
		exit(0);
	}

}

void mem_free()
{	
	free(image_buff);
	free(strip);
}
