#include "transport.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>


struct transport echo_transport;


void signal_handle(int signum)
{
    transport_stop(&echo_transport);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("%s [tcp|udp]:ip:port\n", argv[0]);
        return EXIT_FAILURE;        
    }



    signal(SIGINT, signal_handle);

    transport_init(&echo_transport);

    // 线程开始跑
    // transport_start

    // 
    char *spec = strdup(argv[1]);
    struct iocomponent *ioc = transport_listen(&echo_transport, spec);
    if (ioc == NULL)
    {
        printf("listen errno");
        return;
    }

    transport_wait(&echo_transport);
}

