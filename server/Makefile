BIN = towerserver
CC = gcc
CFLAGS = -Wall -pthread
CLIBS = -ldl
OBJS = mongoose.o

all: $(BIN).c $(BIN).o $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(BIN).o $(OBJS) $(CLIBS)

clean:
	rm -f $(BIN) $(BIN).o $(OBJS)
