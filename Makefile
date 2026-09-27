# WCM demo firmware - host build of the unit tests.
#
#   make test            build + run all unit tests, write build/junit.xml
#   make test DEMO_BUG=1 same, with the deliberate thermal off-by-one enabled
#   make clean

CC      ?= cc
BUILD   := build
DEMO_BUG ?=
CFLAGS  ?= -O1 -g
CFLAGS  += -std=c99 -Wall -Wextra -Werror -pedantic -Iinclude -Itest
ifneq ($(strip $(DEMO_BUG)),)
CFLAGS  += -DWPC_DEMO_THERMAL_BUG=$(DEMO_BUG)
endif

SRCS      := $(wildcard src/*.c)
TEST_SRCS := $(wildcard test/*.c)
OBJS      := $(patsubst %.c,$(BUILD)/%.o,$(SRCS) $(TEST_SRCS))
RUNNER    := $(BUILD)/unit_tests
JUNIT     := $(BUILD)/junit.xml

.PHONY: all test clean
all: $(RUNNER)

$(BUILD)/%.o: %.c $(wildcard include/*.h test/*.h) $(BUILD)/.flags
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Rebuild everything when CFLAGS (e.g. DEMO_BUG) change.
$(BUILD)/.flags: FORCE
	@mkdir -p $(BUILD)
	@echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

$(RUNNER): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

test: $(RUNNER)
	./$(RUNNER) $(JUNIT)

clean:
	rm -rf $(BUILD)

.PHONY: FORCE
FORCE:
