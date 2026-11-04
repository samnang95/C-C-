CC := cc
APP_NAME := mini_shell
SRC_DIR := src
TEST_DIR := tests
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
TEST_BIN_DIR := $(BUILD_DIR)/tests
MOD_DIRS := $(patsubst %/.,%,$(wildcard $(SRC_DIR)/*/.))
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -I$(SRC_DIR) $(addprefix -I,$(MOD_DIRS)) -MMD -MP
TARGET := $(BUILD_DIR)/$(APP_NAME)
SRCS := $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
LIB_OBJS := $(filter-out $(OBJ_DIR)/main.o,$(OBJS))
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/tests/%.o,$(TEST_SRCS))
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(TEST_BIN_DIR)/%,$(TEST_SRCS))
DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)
.PHONY: all run test clean
.SECONDARY: $(OBJS) $(TEST_OBJS)
all: $(TARGET)
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $^ -o $@
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
$(TEST_BIN_DIR)/%: $(OBJ_DIR)/tests/%.o $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@
run: $(TARGET)
	./$(TARGET)
test: $(TEST_BINS)
	@for bin in $(TEST_BINS); do ./$$bin || exit 1; done
clean:
	rm -rf $(OBJ_DIR) $(TEST_BIN_DIR) $(TARGET)
-include $(DEPS)
