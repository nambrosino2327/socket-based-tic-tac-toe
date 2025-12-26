CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I include/

SRC = $(wildcard src/*.cpp)
CORE_OBJS = \
	obj/protocol.o \
	obj/game.o \
	obj/utils.o

.PHONY: all test clean

# Main build
all: bin/server bin/client test

bin/server: obj/server.o $(CORE_OBJS) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

bin/client: obj/client.o $(CORE_OBJS) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

obj/%.o: src/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj bin:
	mkdir -p $@

# Tests
test: bin/test_protocol

bin/test_protocol: obj/test_protocol.o $(CORE_OBJS) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@
	
# Clean
clean:
	rm -f obj/*.o bin/*
