CC           := cc
CFLAGS       := -Wall -Wextra -std=c11 -g -Iinclude -MMD -MP
LDFLAGS      := -lm

APP_NAME     := calculator
SRC_DIR      := src
INC_DIR      := include
TEST_DIR     := tests
BUILD_DIR    := build
OBJ_DIR      := $(BUILD_DIR)/obj
TEST_BIN_DIR := $(BUILD_DIR)/tests

TARGET       := $(BUILD_DIR)/$(APP_NAME)

SRCS         := $(wildcard $(SRC_DIR)/*.c)
OBJS         := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
LIB_OBJS     := $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

TEST_SRCS    := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS    := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/tests/%.o,$(TEST_SRCS))
TEST_BINS    := $(patsubst $(TEST_DIR)/%.c,$(TEST_BIN_DIR)/%,$(TEST_SRCS))

DEPS         := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

.PHONY: all run test clean
.SECONDARY: $(OBJS) $(TEST_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/tests
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN_DIR)/%: $(OBJ_DIR)/tests/%.o $(LIB_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_BINS)
	@if [ -z "$(TEST_BINS)" ]; then \
		echo "No tests found in $(TEST_DIR)/"; \
	else \
		for bin in $(TEST_BINS); do \
			echo "Running $$bin..."; \
			./$$bin || exit 1; \
		done; \
	fi

clean:
	rm -rf $(OBJ_DIR) $(TEST_BIN_DIR) $(TARGET)

-include $(DEPS)
