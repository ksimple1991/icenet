#ifndef ICENET_PACKET_STREAMER_H
#define ICENET_PACKET_STREAMER_H

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct packet;


struct packet_streamer
{
    bool exist_packet_header;
    
    struct packet* (*decode)(struct data_buffer *input);
    bool (*encode)(struct packet *packet, struct data_buffer *output);
};


extern struct packet_streamer deafult_packet_streamer;


#ifdef __cplusplus
}
#endif

#endif /* ICENET_PACKET_STREAMER_H */