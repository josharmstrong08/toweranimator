BIN = towerserver
CC = gcc
CFLAGS = -Wall -pthread -DUSE_WEBSOCKET `pkg-config --cflags jansson` -ggdb
CLIBS = -ldl `pkg-config --libs jansson`
OBJS = mongoose.o

all: $(BIN).c $(BIN).o $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(BIN).o $(OBJS) $(CLIBS)

clean:
	rm -f $(BIN) $(BIN).o $(OBJS)
