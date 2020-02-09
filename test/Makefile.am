AM_CFLAGS=-I$(top_srcdir)/src
LDADD=$(top_srcdir)/src/.libs/libicenet.a
AM_LDFLAGS=-lpthread -lrt

noinst_PROGRAMS=echoserver echoclient
echoserver_SOURCES=echo_server.c
echoclient_SOURCES=echo_client.c
