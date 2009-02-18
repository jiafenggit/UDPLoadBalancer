#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "macro.h"

struct sockaddr_in *select_path(config_t * config, struct sockaddr_in *to)
{
    to->sin_family = AF_INET;
    to->sin_addr.s_addr = inet_addr(HOST);
    to->sin_port = htons(config->port[0]);
    return to;
}

void manage_ack(packet_t * lastSent)
{
    free(lastSent);
}

void manage_nack(int socketfd, packet_t * lastSent, config_t * config)
{
    struct sockaddr_in to;
    send_voice_pkts(socketfd, lastSent, SOCK_DGRAM,
                    select_path(config, &to));
}

unsigned int pa_cpy_to_pp(char *pp, struct packet_additions_t *pa)
{
    int i;
    unsigned int n = 0;
    if (pa->n > 0) {
        memcpy(pp, &pa->n, sizeof(pa->n));
        n = sizeof(pa->n);
        for (i = 0; i < pa->n; i++) {
            memcpy((char *) pp + n, &pa->port[i], sizeof(pa->port[i]));
            n += sizeof(pa->port[i]);
        }
    }
    return n;
}

unsigned int pa_cpy_from_pp(struct packet_additions_t *pa, char *pp)
{
    int i;
    unsigned int n = 0;
    memcpy(&pa->n, pp, sizeof(pa->n));
    warnx("%u ports", pa->n);
    if (pa->n > 0) {
        n = sizeof(pa->n);
        for (i = 0; i < pa->n; i++) {
            memcpy(&pa->port[i], (char *) pp + n, sizeof(pa->port[i]));
            n += sizeof(pa->port[i]);
        }
    }
    return n;
}
