/*
 * sender.cpp
 *
 *  Created on: 2015. 5. 1.
 *      Author: nini
 */

#include "rtp.h"						//include to rtp.h
#include "imageproc.h"				//include to rtp.h

int main(int argc, const char * argv[]) {		//The value of the parameter is transmitted to the string.
	int sock;									//sock variable declaration
	struct sockaddr_in local;				//Declaration of a local variable to the structure
	struct sockaddr_in to;					//Declaration of a to variable to the structure
	u_int32 rtp_stream_address;				//rtp_stream_address
	char *image_data;							//declaration a pointer variable of char type
	char *blocks;								//declaration a pointer variable of char type
	unsigned long data_length;				//data_length declaration
	//change payload_size to send two blocks at a time
	unsigned payload_size = BLOCK_SIZE * BLOCK_SIZE * CHANNELS + sizeof(unsigned);
	struct image_header *img_header;			//Declaration of a img_header variable to the structure

	image_data = readImage("/home/nini/다운로드/flowers.pbm", &data_length);
	img_header = (struct image_header *) image_data;

	if (argc != 3) {
		printf("\nUsage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	// initialize RTP stream address */
	rtp_stream_address = inet_addr(argv[1]);

	// if we got a valid RTP stream address... */
	if (rtp_stream_address != 0) {
		sock = socket(AF_INET, SOCK_DGRAM, 0); // create new socket for sending datagrams
		if (sock >= 0) {
			// prepare local address
			memset(&local, 0, sizeof(local));
			local.sin_family = AF_INET;
			local.sin_port = htons(INADDR_ANY);
			local.sin_addr.s_addr = htonl(INADDR_ANY);

			// bind to local address
			if (bind(sock, (struct sockaddr *) &local, sizeof(local)) == 0) {
				// prepare RTP stream address
				memset(&to, 0, sizeof(to));
				to.sin_family = AF_INET;
				to.sin_port = htons(atoi(argv[2]));
				to.sin_addr.s_addr = rtp_stream_address;

				rgb2ycbcr(image_data + sizeof(image_header), img_header->cols, img_header->rows); //Convert RGB to YCbCr

				if (strcmp(img_header->format, "P5") == 0) {
					blocks = getBlocks(image_data, img_header->cols,
							img_header->rows, 1, BLOCK_SIZE, &data_length);
				} else {
					blocks = getBlocks(image_data, img_header->cols,
							img_header->rows, 3, BLOCK_SIZE, &data_length);
				}

				//send RTP packets
				rtp_send_packets(sock, &to, blocks, data_length, payload_size);
			}
			close(sock);
		}
	}

	free(image_data);	//releases the memory area allocated to the image_data.

	return 0;
}
