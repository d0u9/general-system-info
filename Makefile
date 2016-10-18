CC := gcc
CFLAGS += -Wall -std=gnu99
DLIB := libgensysinfo.so
SLIB := libgensysinfo.a

BUILD_DIR := build
SUB_DIRS := cpu network
VPATH = cpu:network

SRCS := $(foreach dir, $(SUB_DIRS), $(wildcard $(dir)/*.c))
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))
DEPS := $(patsubst %.o, %.d, $(OBJS))

.PHONY: release
release: DEBUG_FLAGS = -O2
release: all

.PHONY: debug
debug: DEBUG_FLAGS = -g -O -DDEBUG
debug: all

-include $(DEPS)

.PHONY: all
all: $(DLIB) $(SLIB)

$(SLIB): $(OBJS)
	ar -cvq $@ $^

$(DLIB): $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -shared -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -MMD -fPIC -c -o $@ $<

$(OBJS): | $(BUILD_DIR)

.PHONY: $(BUILD_DIR)
$(BUILD_DIR):
	@test -d $(BUILD_DIR) || ( mkdir $(BUILD_DIR); echo "mkdir $(BUILD_DIR)" )

.PHONY: clean
clean:
	rm -fr $(BUILD_DIR)
	rm -fr $(DLIB)
	rm -fr $(SLIB)
