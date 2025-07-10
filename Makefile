# Compiler flags
CC := gcc
CFLAGS := -Wall -Wextra -g -Iinclude

# Directories and files
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := test

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c, $(BUILD_DIR)/%.o, $(TEST_SRCS))
TEST_BIN  := test_sh

TARGET := flowsh

# --- Main target ---

# compile the .c files under src
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# link the main program
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: all
all: $(TARGET)

# --- Tests ---

# compile the .c files under test
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# link the test .o files
$(TEST_BIN): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: test
test: $(TEST_BIN)
	./$(TEST_BIN)


# --- Clean up ---
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
 
