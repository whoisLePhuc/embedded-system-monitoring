# Compiler & flags
CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -Iinc
LDFLAGS := 

# Directories
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := build

# Target executable
TARGET  := main

# Source & object files
SRC     := $(wildcard $(SRC_DIR)/*.c) main.c
OBJ     := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))

# Default build
.PHONY: all clean run debug	

all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile .c to .o (tạo cả thư mục con trong build nếu cần)
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Run program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
