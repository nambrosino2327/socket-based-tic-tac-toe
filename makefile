CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Iinclude
DEPFLAGS := -MMD -MP
OBJ_DIR  := obj
BIN_DIR  := bin

# 1. Define your shared logic (no main() functions here)
CORE_SRCS := src/protocol.cc src/game.cc src/utils.cc
CORE_OBJS := $(CORE_SRCS:src/%.cc=$(OBJ_DIR)/%.o)

.PHONY: all clean

all: $(BIN_DIR)/server $(BIN_DIR)/client

# 2. Linking rules: each binary gets its specific .o + all core .os
$(BIN_DIR)/server: $(OBJ_DIR)/server.o $(CORE_OBJS) | $(BIN_DIR)
	@echo "[LD]  $^ --> $@"
	@$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR)/client: $(OBJ_DIR)/client.o $(CORE_OBJS) | $(BIN_DIR)
	@echo "[LD]  $^ --> $@"
	@$(CXX) $(CXXFLAGS) $^ -o $@

# 3. Generic compilation rule
$(OBJ_DIR)/%.o: src/%.cc | $(OBJ_DIR)
	@echo "[CXX] $< --> $@"
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

-include $(OBJ_DIR)/*.d

clean:
	@$(RM) -rv $(OBJ_DIR) $(BIN_DIR)