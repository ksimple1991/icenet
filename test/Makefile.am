AM_CPPFLAGS=-I$(top_srcdir)/src
LDADD=$(top_srcdir)/src/.libs/libicenet.a
AM_LDFLAGS=-lpthread -lrt

noinst_PROGRAMS=echoserver echoclient
echoserver_SOURCES=echoserver.c
echoclient_SOURCES=echoclient.c
