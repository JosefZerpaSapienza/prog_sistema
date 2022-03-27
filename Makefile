#

CC = gcc
ALL = server client
ARGS = -l
# Handle cross-platform compilation with libraries 
UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
	LIBS = -pthread -lrt -g2
	EXE = 
	BINS = $(ALL)
	RM = rm
else
	LIBS = -lws2_32 -g2
	EXE = .exe
	BINS = *.exe
	RM = del
endif


server$(EXE) : server.c queue.h networking.h security.h threads.h synchronization.h commands.h constants.h connection.h authentication.h timing.h logging.h
	$(CC) -o $@ $< $(LIBS)

client$(EXE) : client.c networking.h security.h commands.h constants.h authentication.h
	$(CC) -o $@ $< $(LIBS) 

all: $(ALL)

clean :
	$(RM) $(BINS) # TODO: Fix on windows

runserver : server$(EXE)
	./$< -s 

runclient : client$(EXE)
	./$< -h 127.0.0.1 -p 8888 $(ARGS)

runclient-l : client$(EXE)
	./$< -h 127.0.0.1 -p 8888 -l

runclient-s : client$(EXE)
	./$< -h 127.0.0.1 -p 8888 -s upload.txt

runclient-e : client$(EXE)
	./$< -h 127.0.0.1 -p 8888 -e echo Hello World !

runclient-d : client$(EXE)
	./$< -h 127.0.0.1 -p 8888 -d upload.txt download.txt 

