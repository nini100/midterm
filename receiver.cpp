/*
 * receiver.cpp
 *
 *  Created on: 2015. 5. 1.
 *      Author: nini
 */

#include "rtp.h"
#include "imageproc.h"

void assemblePackets(u_int8 *rcv_data, unsigned long length, char *image_data) {
	int byte_block = 0;
	unsigned *sq_number = (unsigned *) rcv_data;
	byte_block += sizeof(unsigned);
	char *payload = rcv_data + sizeof(unsigned);
	unsigned x_bl, y_bl;
	//printf("%2d: %c\n", *sq_number, *(sq_number + sizeof(unsigned)));
	int k;
	x_bl = (*sq_number * BLOCK_SIZE) % IMAGE_WIDTH;
	y_bl = BLOCK_SIZE * ((*sq_number * BLOCK_SIZE) / IMAGE_WIDTH);
//    printf("<%d> <%d>\n", y_bl, x_bl);
	for (k = 0; k < BLOCK_SIZE; k++) {
		byte_block += BLOCK_SIZE * CHANNELS;
		memcpy(
				image_data + y_bl * IMAGE_WIDTH * CHANNELS + x_bl * CHANNELS
						+ k * IMAGE_WIDTH * CHANNELS,
				payload + k * BLOCK_SIZE * CHANNELS, BLOCK_SIZE * CHANNELS);
	}
//    printf("%d\n", byte_block);
}

int main(int argc, const char * argv[]) {
	int sock;
	struct sockaddr_in local;
	int timeout;
	char *image_data;
	image_header *image_hdr;

	if (argc != 2) {
		printf("Usage: %s <port>", argv[0]);
		exit(1);
	}
	// For now we assume we received header somehow (Later we will get it via RTCP)
	image_data = (char *) malloc(
	IMAGE_WIDTH * IMAGE_HEIGHT * CHANNELS + sizeof(struct image_header)); //It is assigned a memory as given.
	image_hdr = (struct image_header *) image_data;
	image_hdr->cols = IMAGE_WIDTH; //Insert IMAGE_WIDTH in the variables of the structure.
	image_hdr->rows = IMAGE_HEIGHT;
	image_hdr->levels = 255;
	strcpy(image_hdr->format, "P6 "); //change appropriately

	// create new socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock >= 0) {
		// prepare local address
		memset(&local, 0, sizeof(local));
		local.sin_family = AF_INET;
		local.sin_port = htons(atoi(argv[1]));
		local.sin_addr.s_addr = htonl(INADDR_ANY);

		// bind to local address
		if (bind(sock, (struct sockaddr *) &local, sizeof(local)) == 0) {
			// set recv timeout
			timeout = RTP_RECV_TIMEOUT;
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
					sizeof(timeout));

			rtp_recv_packets(sock, assemblePackets,
					image_data + sizeof(image_header));
		}
		close(sock);
	}

	ycbcr2rgb(image_data + sizeof(image_header), image_hdr->cols,
			image_hdr->rows); // Convert YCbCr to RGB
	writeImage("/home/nini/다운로드/flowers_final.pbm", image_data); //Use writeImage method
	return 0;
}
