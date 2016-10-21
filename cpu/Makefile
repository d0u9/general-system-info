CC := gcc
BIN := cpu.out

CFLAGS += -Wall -std=gnu99 -I../trilib -I..
BUILD_DIR := build

SRCS := $(wildcard *.c) $(wildcard sample/*.c) $(wildcard ../trilib/*.c) $(wildcard ../*.c)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))
DEPS := $(patsubst %.o, %.d, $(OBJS))

vpath %.c sample:../trilib:..
vpath %.h sample:../trilib:..

.PHONY: release
release: DEBUG_FLAGS = -O2
release: all

.PHONY: debug
debug: DEBUG_FLAGS = -O -g -DDEBUG
debug: all

-include $(DEPS)

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $(BIN) $^

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -fPIC -MMD -c -o $@ $<

$(OBJS): | $(BUILD_DIR)

.PHONY: $(BUILD_DIR)
$(BUILD_DIR):
	@test -d $(BUILD_DIR) || ( mkdir $(BUILD_DIR); echo "mkdir $(BUILD_DIR)"; )

.PHONY: clean
clean:
	rm -fr $(BUILD_DIR)
	rm -fr $(BIN)



