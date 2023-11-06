CC = g++
CFLAGS = -Wall
LDFLAGS = -pthread

SRCS = server.cpp functions.cpp serverHandler.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = server.h
EXEC = server

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean
