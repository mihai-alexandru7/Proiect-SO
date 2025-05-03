# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

# Folders
TREASURE_MANAGER_DIR = treasure_manager
TREASURE_HUB_DIR = treasure_hub
BIN_DIR = bin
OBJ_DIR = obj
HUNT_DIR = hunts
LOG_DIR = logs

# Headers files
TREASURE_MANAGER_HEADERS = $(wildcard $(TREASURE_MANAGER_DIR)/*.h)

# Sources files
TREASURE_MANAGER_SRCS = $(wildcard $(TREASURE_MANAGER_DIR)/*.c)
TREASURE_HUB_SRCS = $(wildcard $(TREASURE_HUB_DIR)/*.c)

# Object files
TREASURE_MANAGER_OBJS = $(patsubst $(TREASURE_MANAGER_DIR)/%.c, $(OBJ_DIR)/%.o, $(TREASURE_MANAGER_SRCS))
TREASURE_HUB_OBJS = $(patsubst $(TREASURE_HUB_DIR)/%.c, $(OBJ_DIR)/%.o, $(TREASURE_HUB_SRCS))

# Executables
TREASURE_MANAGER = $(BIN_DIR)/treasure_manager
TREASURE_HUB = $(BIN_DIR)/treasure_hub

# Remove command
RM = rm -f

# Initialize folders
init:
	@echo "Initializing project"
	@mkdir -p $(BIN_DIR) $(OBJ_DIR) $(HUNT_DIR) $(LOG_DIR)

# Default target
build: $(TREASURE_MANAGER) $(TREASURE_HUB)
	@echo "Build complete"

# Executables
$(TREASURE_MANAGER): $(TREASURE_MANAGER_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^

$(TREASURE_HUB): $(TREASURE_HUB_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^

# Compile rules
$(OBJ_DIR)/%.o: $(TREASURE_MANAGER_DIR)/%.c $(TREASURE_MANAGER_HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TREASURE_HUB_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	@echo "Cleaning project"
	@$(RM) $(TREASURE_MANAGER) $(TREASURE_HUB) $(TREASURE_MANAGER_OBJS) $(TREASURE_HUB_OBJS)
	@rm -rf $(OBJ_DIR)

clean-all: clean
	@rm -rf $(BIN_DIR) $(HUNT_DIR) $(LOG_DIR)

.PHONY: init build clean
