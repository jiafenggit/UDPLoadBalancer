#ifndef __COMM_H__
#define __COMM_H__

#include "packet.h"

char recvMonitorPkts(int, config_t *);
uint32_t recvVoicePkts(int, packet_t *);
void sendVoicePkts(int, packet_t *);
void sendPktsToApp(int, packet_t *, packet_t **, uint32_t *);

#endif
