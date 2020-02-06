#include "packet_streamer.h"

/**
 * iCeN
 */
#define ICENET_PAKCET_FLAG 0x6943654E


struct packet_streamer deafult_packet_streamer =
{
    .exist_packet_header = true;
    .decode = default_packet_streamer_decode,
    .encode = default_packet_streamer_encode
};

struct packet* default_packet_streamer_decode(struct packet_streamer *streamer, struct packet_buffer *input)
{
    
}

bool default_packet_streamer_encode(struct packet_streamer *streamer, struct packet *packet, struct packet_buffer *output)
{

}

struct packet* default_packet_streamer_get_packet_info(struct packet_streamer *streamer, \
    struct packet_buffer *input, struct packet_header *header)
{
    if (streamer->exist_packet_header)
    {
        if (packet_buffer_get_data_len(input) < (int)(4 * sizeof(int)))
        {
            return false;
        }

        int flag = packet_buffer_read_int32(input);
        
    }
}