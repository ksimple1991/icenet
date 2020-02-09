#ifndef ICENET_CONTROL_PACKET_H
#define ICENET_CONTROL_PACKET_H

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    CMD_BAD_PACKET = 1,
    CMD_TIMEOUT_PACKET,
    CMD_DISCONN_PACKET
};

struct control_packet
{
    int command;
};

extern struct control_packet s_bad_packet;
extern struct control_packet s_timeout_packet;
extern struct control_packet s_disconn_packet;

#ifdef __cplusplus
}
#endif

#endif /* ICENET_CONTROL_PACKET_H */
