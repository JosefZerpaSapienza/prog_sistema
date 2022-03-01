#

CC = gcc
ALL = server client
# Handle cross-platform compilation with libraries 
UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
	LIBS = -pthread -lrt
	EXE = 
	BINS = $(ALL)
else
	LIBS = -lws2_32
	EXE = .exe
	BINS = *.exe
endif


server$(EXE) : server.c queue.h networking.h security.h threads.h synchronization.h
	$(CC) -o $@ $< $(LIBS)

client$(EXE) : client.c networking.h security.h
	$(CC) -o $@ $< $(LIBS) 

all: $(ALL)

clean :
	rm $(BINS) # TODO: Fix on windows


runserver : server$(EXE)
	./$< -s 

runclient : client$(EXE)
	./$< -h 127.0.0.1 -p 8888 -l

