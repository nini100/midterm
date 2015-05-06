#ifndef RTPCommunication_rtp_h
#define RTPCommunication_rtp_h

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define u_int8 char
#define u_int16 short
#define u_int32 int

#define	RTP_VERSION                 0x80
#define RTP_PACKET_SIZE             4000
#define RTP_PAYLOADTYPE             96
#define RTP_MARKER_MASK             0x80
#define RTP_SSRC                    0
#define RTP_TIMESTAMP_INCREMENT     3600
#define RTP_RECV_TIMEOUT            2000
#define RTP_RECV_STATS              50

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


//Define RTP header
typedef struct{
    u_int8		flags;			/* Version(2),Padding(1), Ext(1), Cc(4) */
    u_int8		mk_pt;			/* Marker(1), PlayLoad Type(7) */
    u_int16		seqNum;			/* Sequence Number */
    u_int32		timestamp;		/* Timestamp */
    u_int32		ssrc;			/* SSRC */
} rtp_hdr;


void            rtp_send_packets( int sock, struct sockaddr_in* to, char *, unsigned long, unsigned);
unsigned long   rtp_recv_packets(int socket, void (*call_back)(char *, unsigned long length, char *image_data), char *image_data);

#endif
