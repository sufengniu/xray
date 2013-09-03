#include "udp_server.h"

#include "tif.h"

int main(int argc, char *argv[])
{
	int i, j, k;
	int num_packets;
	int packet_size = PACKET_SIZE;

	int ping_pong;

	image_info = (struct image_info_type *)malloc(sizeof(struct image_info_type));	
	
	/* setup udp socket */
	printf("UDP socket setup...\n");
	struct sockaddr_in *client_addr;
	int sid;
	if((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error: socket init failed!\n");
		exit(0);
	}
	client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
		
	client_addr->sin_family = AF_INET;
	client_addr->sin_port = htons(PORT);
	client_addr->sin_addr.s_addr = inet_addr(ipaddr);	
	
	if(bind(sid, (struct sockaddr *)client_addr, sizeof(struct sockaddr)) == -1) {
		printf("Error: bind failed\n");
		exit(0);
	}
	socklen_t size = sizeof(struct sockaddr);
	
	/* hand shaking */	
	struct id_type sendid, recvid;
	sendid.bid = 1;	
	recvfrom(sid, &recvid, sizeof(struct id_type), MSG_WAITALL, (struct sockaddr *)client_addr, &size);
	printf("Recive hand shaking signal from Blade %d\n", recvid.bid);
	
	sendto(sid, &sendid, sizeof(struct id_type), 0, (struct sockaddr *)client_addr, size);

	recvfrom(sid, image_info, sizeof(struct image_info_type), MSG_WAITALL, (struct sockaddr *)client_addr, &size);
	
	num_packets = ceil((float)image_info->image_size/packet_size);
	printf("image info:\n");
	printf("image length: %d, image width: %d\n", image_info->length, image_info->width);
	printf("image page number: %d\n", image_info->page_num);
	printf("packet number per frame: %d\n", num_packets);
	
	mem_alloc();

	ping_pong = 0;	
	image_ptr = image_buff;	
	for (i = 0; i < image_info->page_num; i++)
	{	

		image_ptr = image_buff + i*image_info->image_size;
		
		for (j = 0; j < num_packets; j++)
		{
	
			recvfrom(sid, image_ptr, 2*packet_size, 0, (struct sockaddr *)client_addr, &size);
				
			image_ptr += packet_size;
			
/*			recvfrom(sid, strip, 2*packet_size, 0, (struct sockaddr *)client_addr, &size);
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
			}*/
		}

		printf("Receiving %dth image frame\n", i);
		ping_pong ^= 1;	
	}
	
	printf("Receiving done\n");
	close(sid);
	
	tif_multi_syn(image_info, image_buff, output_filename[2]);
	
	printf("-- finished --\n");

	free(client_addr);
	mem_free();

	return 0;
}

void mem_alloc()
{
	if((image_buff = (uint16 *)malloc(image_info->page_num * image_info->image_size * sizeof(uint16))) == NULL)
	{
		printf("Error: could not allocate enought memory for image_buff!\n");
		exit(0);
	}
/*	if((image_buff = (uint16 *)malloc(2 * image_info->image_size * sizeof(uint16))) == NULL)
	{
		printf("Error: could not allocate enought memory for image_buff!\n");
		exit(0);
	}
*/
	if((strip = (uint16 *)malloc(PACKET_SIZE * sizeof(uint16))) == NULL)
	{
		printf("~: Error: could not allocate enought memory for strip!\n");
		exit(0);
	}
}

void mem_free()
{
	free(image_info);
	free(image_buff);
	free(strip);
}
