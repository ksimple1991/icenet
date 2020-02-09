AM_CFLAGS=
source_list=buffer.c channelpool.c connection.c epollsocketevent.c iocomponent.c isocket.c packet_queue.c packet_streamer.c share_func.c tcp.c transport.c

AM_LDFLAGS="-lpthread -lrt"
# test_sources=
lib_LTLIBRARIES=libicenet.la
libicenet_la_SOURCES=$(source_list)
libicenet_la_LDFLAGS=$(AM_LDFLAGS) -static-libgcc
include_HEADERS=buffer.h channelpool.h connection.h epollsocketevent.h iocomponent.h isocket.h list.h packet_queue.h packet_streamer.h share_func.h tcp.h transport.h util.h

noinst_PROGRAMS=
