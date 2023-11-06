CXX = g++
CXXFLAGS = -Wall -pthread

# Define the source files
SRCS = clientFunc.cpp clientProcess.cpp client.cpp upd.cpp commands.cpp fileHash.cpp

# Create a list of object files by replacing .cpp with .o
OBJS = $(SRCS:.cpp=.o)

# Define the target executable
TARGET = client

# Link with the OpenSSL library
LDLIBS = -lssl -lcrypto

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
