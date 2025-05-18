# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Folders
HEADER_DIR = include
SRC_DIR = src
UTILS_DIR = utils
TREASURE_MANAGER_DIR = treasure_manager
TREASURE_MONITOR_DIR = treasure_monitor
TREASURE_HUB_DIR = treasure_hub
CALCULATE_SCORE_DIR = calculate_score
BIN_DIR = bin
OBJ_DIR = obj
HUNTS_DIR = hunts
LOGS_DIR = logs

# Header files
TREASURE_MANAGER_HEADERS = $(shell find $(HEADER_DIR) -name "treasure*.h") $(shell find $(SRC_DIR)/$(TREASURE_MANAGER_DIR)/utils -name "*.h")
TREASURE_MONITOR_HEADERS = $(HEADER_DIR)/parser.h
TREASURE_HUB_HEADERS = $(HEADER_DIR)/parser.h
CALCULATE_SCORE_HEADERS = $(HEADER_DIR)/treasure.h

# Source files
UTILS_SRCS = $(shell find $(SRC_DIR)/$(UTILS_DIR) -name "*.c")
TREASURE_MANAGER_SRCS = $(shell find $(SRC_DIR)/$(TREASURE_MANAGER_DIR) -name "*.c")
TREASURE_MONITOR_SRCS = $(shell find $(SRC_DIR)/$(TREASURE_MONITOR_DIR) -name "*.c") $(UTILS_SRCS)
TREASURE_HUB_SRCS = $(shell find $(SRC_DIR)/$(TREASURE_HUB_DIR) -name "*.c") $(UTILS_SRCS)
CALCULATE_SCORE_SRCS = $(shell find $(SRC_DIR)/$(CALCULATE_SCORE_DIR) -name "*.c")

# Object files
TREASURE_MANAGER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(TREASURE_MANAGER_SRCS))
TREASURE_MONITOR_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(TREASURE_MONITOR_SRCS))
TREASURE_HUB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(TREASURE_HUB_SRCS))
CALCULATE_SCORE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CALCULATE_SCORE_SRCS))

# Executables
TREASURE_MANAGER = $(BIN_DIR)/treasure_manager
TREASURE_MONITOR = $(BIN_DIR)/treasure_monitor
TREASURE_HUB = $(BIN_DIR)/treasure_hub
CALCULATE_SCORE = $(BIN_DIR)/calculate_score

# Remove command
RM = rm -rf

all: init build

# Initialize target
init:
	@echo "Initializing project..."
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(HUNTS_DIR)
	@mkdir -p $(LOGS_DIR)

# Build target
build: prebuild $(TREASURE_MANAGER) $(TREASURE_MONITOR) $(TREASURE_HUB) $(CALCULATE_SCORE)
	@echo "Build complete"

prebuild:
	@echo "Building project..."

# Linking treasure_manager executable from object files
$(TREASURE_MANAGER): $(TREASURE_MANAGER_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^

# Linking treasure_monitor executable from object files
$(TREASURE_MONITOR): $(TREASURE_MONITOR_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^

# Linking treasure_hub executable from object files
$(TREASURE_HUB): $(TREASURE_HUB_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^

# Linking calculate_score executable from object files
$(CALCULATE_SCORE): $(CALCULATE_SCORE_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/$(UTILS_DIR)/%.o: $(SRC_DIR)/$(UTILS_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile treasure_manager source files
$(OBJ_DIR)/$(TREASURE_MANAGER_DIR)/%.o: $(SRC_DIR)/$(TREASURE_MANAGER_DIR)/%.c $(TREASURE_MANAGER_HEADERS)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile treasure_monitor source files
$(OBJ_DIR)/$(TREASURE_MONITOR_DIR)/%.o: $(SRC_DIR)/$(TREASURE_MONITOR_DIR)/%.c $(TREASURE_MONITOR_HEADERS)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile treasure_hub source files
$(OBJ_DIR)/$(TREASURE_HUB_DIR)/%.o: $(SRC_DIR)/$(TREASURE_HUB_DIR)/%.c $(TREASURE_HUB_HEADERS)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile calculate_score source files
$(OBJ_DIR)/$(CALCULATE_SCORE_DIR)/%.o: $(SRC_DIR)/$(CALCULATE_SCORE_DIR)/%.c $(CALCULATE_SCORE_HEADERS)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CFAGS) -c $< -o $@

# Clean target
clean:
	@echo "Cleaning project..."
	@$(RM) $(BIN_DIR)
	@$(RM) $(OBJ_DIR)
	@echo "Cleaning complete"

.PHONY: all init build clean
