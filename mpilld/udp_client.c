#include "udp_client.h"

#include "tif.h"

int main(int argc, char *argv[])
{
	int i, j;	// i: image index, j: packet index
	int k;
	int packet_size = PACKET_SIZE;
	int num_packets = 0;
	int num_sent;	
	// long int image_offset = 0;

	image_info = (struct image_info_type *)malloc(sizeof(struct image_info_type));	
	
	
	/* loading tif into memory */
	info = tif_info(argv+1);
	
	/* global variablei, buffer length and buffer width will be determined on server side */
	image_info->image_size = info->length * info->width;
	image_info->length = info->length;
	image_info->width = info->width;
	image_info->page_num = info->page_num;
	mem_alloc();
	tif_multi_load(sim_image);		

	/* setup udp socket */
	struct sockaddr_in *server_addr;	
	int sid;	// socket ID
	if (((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1))
	{
		printf("Error: socket initial failed!\n");
		exit(0);
	}
	server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));	
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORT);      // host to network, short

	inet_pton(AF_INET, ipaddr, &server_addr->sin_addr);	
	socklen_t size=sizeof(struct sockaddr_in);
	
	/* send the image */
	printf("---------------------------------\n");
	printf("-- Initial done, ready to send\n");
	printf("---------------------------------\n");
	
	/* hand shaking */	
	struct id_type sendid, recvid;
	sendid.bid = 0;	// blade 0 (CIN card)
	sendto(sid, &sendid, sizeof(struct id_type), 0, (struct sockaddr *)server_addr, size);
	printf("Waiting for server responds...\n");
	
	recvfrom(sid, &recvid, sizeof(struct id_type), 0, (struct sockaddr *)server_addr, &size);

	printf("Receive ACK signal from blade %d, link is OK\n", recvid.bid);
	
	// send image parameters	
	sendto(sid, image_info, sizeof(struct image_info_type), 0, (struct sockaddr *)server_addr, size);
		
	num_packets = ceil((float)image_info->image_size/packet_size);
	printf("num_packets for each frame is %d\n", num_packets);

	usleep(1000000);
		
	for (i = 0; i < image_info->page_num; i++) {
		
		image_ptr = sim_image + i*image_info->image_size;
		printf("Sending the %dth image frame\n", i);	
	
		for (j = 0; j < num_packets; j++) {		
			// each pixel require 2 bytes	
			num_sent = sendto(sid, image_ptr, 2*packet_size, 0, (struct sockaddr *)server_addr, size);
			usleep(100);

			image_ptr += packet_size;

		}

		usleep(1000000);
	}
		
	printf("image sending done!\n");

	/* cleaning */	
	close(sid);

	free(server_addr);
	mem_free();
	printf("-- finished --\n");
	return 0;
}

void mem_alloc()
{
	if((sim_image = (uint16 *)malloc(image_info->page_num * image_info->image_size * sizeof(uint16))) == NULL){
		printf("Error: could not allocate enough memory for sim_image");
		exit(0);
	} 
}

void mem_free()
{
	free(image_info);
	free(sim_image);
}
