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

    transport_init(&echo_transport);

    signal(SIGTERM, signal_handle);
    signal(SIGINT, signal_handle);
    signal(SIGQUIT, signal_handle);

    // 线程开始跑
    echo_server_start(&echo_transport);

}

void echo_server_start(struct transport *echo_server)
{
    transport_start(echo_server);

    // 
    char *spec = strdup(argv[1]);
    struct iocomponent *ioc = transport_listen(echo_server, spec);
    if (ioc == NULL)
    {
        printf("listen errno");
        return;
    }

    transport_wait(echo_transport);
}