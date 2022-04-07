#

CC = gcc
COPTIONS = -Wall -g2
SOURCES_DIR = ./sources/
BIN_DIR = ./bin/
BINS = server client
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
				constants.h logging.h networking.h queue.h \
				security.h synchronization.h threads.h timing.h)
	$(CC) $(COPTIONS) -o $(BIN_DIR)$@ $< $(LIBS)

client$(EXE) : $(addprefix $(SOURCES_DIR), client.c \
				authentication.h commands.h constants.h \
				networking.h security.h)
	$(CC) $(COPTIONS) -o $(BIN_DIR)$@ $< $(LIBS) 

all: $(addsuffix $(EXE), $(BINS))

clean :
	$(RM) $(addsuffix $(EXE), $(BINS)) 

runserver : server$(EXE)
	$(BIN_DIR)$< -s 

runclient : client$(EXE)
	$(BIN_DIR)$< -h 127.0.0.1 -p 8888 $(ARGS)

runclient-l : client$(EXE)
	$(BIN_DIR)$< -h 127.0.0.1 -p 8888 -l

runclient-s : client$(EXE)
	$(BIN_DIR)$< -h 127.0.0.1 -p 8888 -s misc/upload.txt

runclient-e : client$(EXE)
	$(BIN_DIR)$< -h 127.0.0.1 -p 8888 -e echo Hello World !

runclient-d : client$(EXE)
	$(BIN_DIR)$< -h 127.0.0.1 -p 8888 -d misc/upload.txt temp/download.txt 

#
echo : 
	echo $(addsuffix $(SOURCES_DIR), server.c \
				authentication.h commands.h connnection.h \
				constants.h logging.h networking.h queue.h \
				security.h synchronization.h threads.h timing.h)

test : $(addprefix $(SOURCES_DIR), test.c test.h)
	gcc -o $(BIN_DIR)$@ $<

runtest : test
	$(BIN_DIR)$<
	
	
