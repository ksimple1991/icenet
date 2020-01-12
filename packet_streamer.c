#include "packet_streamer.h"

/**
 * iCeN
 */
#define ICENET_PAKCET_FLAG 0x6943654E


struct packet_streamer deafult_packet_streamer =
{
    .exist_ packet_header = true;
    .decode = default_packet_streamer_decode,
    .encode = default_packet_streamer_encode
};

struct packet* default_packet_streamer_decode(struct data_buffer *input)
{
    
}

bool default_packet_streamer_encode(struct packet *packet, struct data_buffer *output)
{

}
