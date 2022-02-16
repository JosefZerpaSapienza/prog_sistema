#

CC = gcc
# Handle cross-platform compilation with libraries 
UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
	LIBS = -pthreads -lrt
else
	LIBS = -lws2_32
endif


server : server.c queue.h networking.h security.h threads.h synchronization.h
	$(CC) -o $@ $< $(LIBS)

client : client.c networking.h security.h
	$(CC) -o $@ $< 


all: server client

rserver : server
	./$< 

rclient : client
	./$< -h 127.0.0.1 -p 8888 -l
