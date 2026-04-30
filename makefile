CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I include/

SRC = $(wildcard src/*.cc)
CORE_OBJS = \
	obj/protocol.o \
	obj/game.o \
	obj/utils.o

.PHONY: all clean

# Main build
all: bin/server bin/client

bin/%: obj/%.o $(CORE_OBJS) | bin
	@echo "[CXX] $^ --> $@"
	@$(CXX) $(CXXFLAGS) $^ -o $@

obj/%.o: src/%.cc | obj
	@echo "[CXX] $^ --> $@"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

obj bin:
	mkdir -p $@
	
# Clean
clean:
	@echo "Cleaning up..."
	@rm -f obj/*.o bin/*
