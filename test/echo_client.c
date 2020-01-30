#include "transport.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

struct transport echo_client;
int gsendcount = 0;

void signal_handle(int sig_num)
{
    transport_stop(&echo_client);
}

int void main(int argc,char *argv[])
{
    if (argc != 4)
    {
        printf("%s [tcp|udp]:ip:port count conn\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sendcount = atoi(argv[2]);
    if (sendcount > 0)
    {
        gsendcount = sendcount;
    }

    int conncount = atoi(argv[3]);
    if (conncount < 1)
    {
        conncount = 1;
    }

    signal(SIGINT, signal_handle);
    signal(SIGTERM, signal_handle);

    int64_t start_time = time(NULL);

    transport_init(&echo_client);

    char *spec = strdup(argv[1]);
    if (spec == NULL)
    {
        return EXIT_FAILURE;
    }

    transport_connect(&echo_client, spec, false);


    free(spec);
    return EXIT_SUCCESS;
}