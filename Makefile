CC = gcc
CFLAGS = -Wall -pthread
TARGET = ghost_sim
OBJS = main.o helpers.o room.o hunter.o ghost.o evidence.o house.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c defs.h helpers.h
	$(CC) $(CFLAGS) -c main.c

helpers.o: helpers.c helpers.h defs.h
	$(CC) $(CFLAGS) -c helpers.c

room.o: room.c defs.h helpers.h
	$(CC) $(CFLAGS) -c room.c

hunter.o: hunter.c defs.h helpers.h
