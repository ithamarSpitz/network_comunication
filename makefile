CC = gcc
CFLAGS = -Wall -Wextra -std=c99

RUDP_SOURCES = rudp/RUDP_API.c rudp/RUDP_Sender.c rudp/RUDP_Receiver.c
TCP_SOURCES = tcp/TCP_Sender.c tcp/TCP_Receiver.c

RUDP_OBJECTS = $(RUDP_SOURCES:.c=.o)
TCP_OBJECTS = $(TCP_SOURCES:.c=.o)

RUDP_EXECUTABLES = RUDP_Sender RUDP_Receiver
TCP_EXECUTABLES = TCP_Sender TCP_Receiver

all: $(RUDP_EXECUTABLES) $(TCP_EXECUTABLES)

RUDP_Sender: $(RUDP_OBJECTS)
	$(CC) $(CFLAGS) -o RUDP_Sender rudp/RUDP_API.o rudp/RUDP_Sender.o

RUDP_Receiver: $(RUDP_OBJECTS)
	$(CC) $(CFLAGS) -o RUDP_Receiver rudp/RUDP_API.o rudp/RUDP_Receiver.o

TCP_Sender: $(TCP_OBJECTS)
	$(CC) $(CFLAGS) -o TCP_Sender tcp/TCP_Sender.o

TCP_Receiver: $(TCP_OBJECTS)
	$(CC) $(CFLAGS) -o TCP_Receiver tcp/TCP_Receiver.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(RUDP_OBJECTS) $(TCP_OBJECTS) $(RUDP_EXECUTABLES) $(TCP_EXECUTABLES)
