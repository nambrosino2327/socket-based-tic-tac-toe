CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I include/

SRC = $(wildcard src/*.cpp)
CORE_OBJS = \
	obj/protocol.o \
	obj/game.o \
	obj/utils.o

.PHONY: all clean

# Main build
all: bin/server bin/client

bin/%: obj/%.o $(CORE_OBJS) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

obj/%.o: src/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj bin:
	mkdir -p $@
	
# Clean
clean:
	rm -f obj/*.o bin/*
