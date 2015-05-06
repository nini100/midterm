/*
 * rtp.cpp
 *
 *  Created on: 2015. 5. 1.
 *      Author: nini
 */

#include "rtp.h"

static unsigned timestamp;

//  send RTP packets
void rtp_send_packets(int sock, struct sockaddr_in* to, char *rtp_data, unsigned long length, unsigned payload_size){
    rtp_hdr* rtphdr;
    char*           rtp_payload;
    u_int32             rtp_payload_size, rtp_data_index;
    u_int8 *rtp_send_packet = (u_int8 *) malloc(payload_size);

    // prepare RTP packet
    rtphdr = (rtp_hdr*) rtp_send_packet;
    rtphdr->flags = RTP_VERSION;
    rtphdr->mk_pt = RTP_PAYLOADTYPE;
    rtphdr->ssrc  = htonl(RTP_SSRC);
    timestamp += 1000/30;
    rtphdr->timestamp   = htonl(ntohl(timestamp));

    // send RTP stream packets
    rtp_data_index = 0;
    do {
        memset(rtp_send_packet, 0, sizeof(rtp_send_packet));
        rtp_payload      = rtp_send_packet + sizeof(rtp_hdr);
        rtp_payload_size = MIN(payload_size , (length - rtp_data_index));

        memcpy(rtp_payload, rtp_data + rtp_data_index, rtp_payload_size);

        // set MARKER bit in RTP header on the last packet of an image
        rtphdr->mk_pt = RTP_PAYLOADTYPE | (((rtp_data_index + rtp_payload_size)
                                                  >= length) ? RTP_MARKER_MASK : 0);


        //send RTP stream packet
        if (sendto(sock, rtp_send_packet, sizeof(rtp_hdr) + rtp_payload_size,
                   0, (struct sockaddr *)to, sizeof(struct sockaddr)) >= 0) {
            rtphdr->seqNum  = htons(ntohs(rtphdr->seqNum) + 1);
            rtp_data_index += rtp_payload_size;
        } else {
            printf("rtp_sender: not sendto==%i\n", errno);
        }
    }while (rtp_data_index < length);

    free(rtp_send_packet);
}

//  receives RTP packets
unsigned long rtp_recv_packets(int socket, void (*call_back)(char *, unsigned long length, char *), char *image_data){
    rtp_hdr*    rtphdr;
    struct sockaddr_in from;
    int fromlen;

    u_int32            received_num_bytes;
    u_int32            recvrtppackets  = 0;
    u_int32            lostrtppackets  = 0;
    u_int16            lastrtpseq = 0;
    u_int8 rtp_recv_packet[RTP_PACKET_SIZE];
    unsigned long       total_received_bytes = 0;

    do {
        fromlen = sizeof(from);
        received_num_bytes  = recvfrom(socket, rtp_recv_packet, sizeof(rtp_recv_packet), 0,
                           (struct sockaddr *)&from, (socklen_t *)&fromlen);
        if (received_num_bytes >= sizeof(rtp_hdr)) {
            rtphdr = (rtp_hdr *)rtp_recv_packet;
            recvrtppackets++;
            if ((lastrtpseq == 0) || ((lastrtpseq + 1) == ntohs(rtphdr->seqNum))) {
                total_received_bytes += received_num_bytes;
                call_back((rtp_recv_packet + sizeof(rtp_hdr)),(received_num_bytes - sizeof(rtp_hdr)), image_data);
            } else {
                lostrtppackets++;
            }
            lastrtpseq = ntohs(rtphdr->seqNum);
            if ((recvrtppackets % RTP_RECV_STATS) == 0) {
                printf("rtp_recv_thread: recv %6i packet(s) / lost %4i packet(s) (%.4f%%)...\n", recvrtppackets, lostrtppackets, (lostrtppackets*100.0)/recvrtppackets);
            }
        } else {
            printf("rtp_recv_thread: recv timeout...\n");
            return total_received_bytes;
        }
    }while(!(rtphdr->mk_pt & RTP_MARKER_MASK)); //repeat until end marker is received
    return total_received_bytes;
}


