#include <string.h>             /* memset */
#include <err.h>
#include <stdlib.h>             /* malloc */
#include "macro.h"

#define NFDS 3
#define NPKTS 2
#define APP_PORT 11001
#define PEER_PORT 10001
#define PATH_PORT 9001
#define FIRST_PACKET_ID 0
#define MAX_RECVQ_LENGTH 2

typedef enum types {
    app, peer, path
} types;
int main(int argc, char *argv[])
{
    int i, socks, maxfd, fd[NFDS], recvq_length;
    uint32_t expected_pkt;
    struct sockaddr_in from;
    packet_t *pkt[NPKTS], *recvq;
    fd_set infds, allsetinfds;
    char peerloss;

    expected_pkt = FIRST_PACKET_ID;
    peerloss = 0;

    recvq = NULL;
    recvq_length = 0;

    for (i = 0; i < NPKTS; i++)
        pkt[i] = NULL;

    FD_ZERO(&allsetinfds);

    fd[app] = accept_app(listen_app(HOST, APP_PORT));
    FD_SET(fd[app], &allsetinfds);

    fd[peer] = bind_udp(HOST, PEER_PORT);
    FD_SET(fd[peer], &allsetinfds);

    maxfd = fd[peer];

    for (;;) {
        infds = allsetinfds;
        socks = select(maxfd + 1, &infds, NULL, NULL, NULL);
        if (socks <= 0)
            err(ERR_SELECT);
        if (recvq_length >= MAX_RECVQ_LENGTH) {
            while (recvq_length > 0) {
                packet_t *a_pack = q_extract_first(&recvq);
                recvq_length--;
                if (a_pack->id >= expected_pkt) {
                    send_voice_pkts(fd[app], a_pack, SOCK_STREAM, NULL);
                    expected_pkt = a_pack->id + 1;
                }
                free(a_pack);
            }
        }
        while (socks > 0) {
            socks--;
            if (FD_ISSET(fd[peer], &infds)) {
                uint32_t pktid;
                pkt[peer] = (packet_t *) malloc(sizeof(packet_t));
                pktid =
                    recv_voice_pkts(fd[peer], pkt[peer], SOCK_DGRAM,
                                    &from);
                peerloss = pkt[peer]->pa.ploss;
                if (pktid == expected_pkt) {
                    send_voice_pkts(fd[app], pkt[peer], SOCK_STREAM, NULL);
                    free(pkt[peer]);
                    expected_pkt++;
                } else if (pktid > expected_pkt) {
                    recvq_length += q_insert(&recvq, pkt[peer]);
                }
#ifdef DEBUG
                warnx("received pkt %u, expected %u", pktid, expected_pkt);
#endif
                FD_CLR(fd[peer], &infds);
            } else if (FD_ISSET(fd[app], &infds)) {
                unsigned int resend_rate;
                if (peerloss > 0) {
                    resend_rate = (25 / peerloss) + 1;
                    warnx("resend rate = %u", resend_rate);
                    if (random() % resend_rate == 1)
                        send_voice_pkts(fd[peer], pkt[app], SOCK_DGRAM,
                                        &from);
                }
                free(pkt[app]);
                pkt[app] = (packet_t *) malloc(sizeof(packet_t));
                recv_voice_pkts(fd[app], pkt[app], SOCK_STREAM, NULL);
                send_voice_pkts(fd[peer], pkt[app], SOCK_DGRAM, &from);
                FD_CLR(fd[app], &infds);
            } else
                errx(ERR_NOFDSET);
        }
    }
    return 127;
}
