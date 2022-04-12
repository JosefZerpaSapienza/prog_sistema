#

CC = gcc
COPTIONS = -Wall -g2
SOURCES_DIR = ./sources/
BIN_DIR = ./bin/
BINS = server client
IP = 127.0.0.1 
PORT = 8888
ARGS = -l

# Handle cross-platform compilation with libraries 
UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
	LIBS = -pthread -lrt
	EXE = 
	RM = rm
else
	LIBS = -lws2_32
	EXE = .exe
	RM = del
endif


# Take sources from sources/ and compile binaries in bin/
server$(EXE) : $(addprefix $(SOURCES_DIR), server.c \
				authentication.h commands.h connection.h \
				constants.h daemonize.h logging.h networking.h \
				queue.h security.h synchronization.h threads.h \
				timing.h)
	$(CC) $(COPTIONS) -o $(BIN_DIR)$@ $< $(LIBS)

client$(EXE) : $(addprefix $(SOURCES_DIR), client.c \
				authentication.h commands.h constants.h \
				networking.h security.h)
	$(CC) $(COPTIONS) -o $(BIN_DIR)$@ $< $(LIBS) 

all: $(addsuffix $(EXE), $(BINS))

clean :
	$(RM) $(addsuffix $(EXE), $(BINS)) 

runserver : server$(EXE)
	$(BIN_DIR)$< -p $(PORT)

runclient : client$(EXE)
	$(BIN_DIR)$< -h $(IP) -p $(PORT) $(ARGS)

runclient-l : client$(EXE)
	$(BIN_DIR)$< -h $(IP) -p $(PORT) -l sources/

runclient-s : client$(EXE)
	$(BIN_DIR)$< -h $(IP) -p $(PORT) -s misc/upload.txt

runclient-e : client$(EXE)
	$(BIN_DIR)$< -h $(IP) -p $(PORT) -e echo Hello World!

runclient-d : client$(EXE)
	$(BIN_DIR)$< -h $(IP) -p $(PORT) -d misc/hello.txt temp/download.txt 

runclient-u : client$(EXE)
	$(BIN_DIR)$< -h $(IP) -p $(PORT) -u misc/relazione.pdf temp/relazione.pdf 

