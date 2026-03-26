#
# Toolchain
#
PREFIX =
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
CP = $(PREFIX)objcopy
OD = $(PREFIX)objdump
SZ = $(PREFIX)size

#
# Paths
#
PROJ_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
TEST_DIR = $(PROJ_DIR)
FW_DIR = $(PROJ_DIR)/../crossfirmarizer/Core
BUILD_DIR = $(TEST_DIR)/build

#
# Target settings
#
TARGET = $(BUILD_DIR)/crossfirmarizer_simulation

#
# Source files
#
C_SOURCES = $(wildcard $(TEST_DIR)/src/*.c) \
			$(wildcard $(TEST_DIR)/src/hal/*.c) \
		    $(wildcard $(FW_DIR)/Src/*.c)
C_SOURCES := $(filter-out $(FW_DIR)/Src/main.c,$(C_SOURCES))

#
# Include paths
#
C_INCLUDES = -I$(FW_DIR)/Inc \
			 -I$(TEST_DIR)/inc

#
# Compiler flags
#
USER_DEFINES = -DTARGET_STM32F446RE -DULTRASONIC_SENSOR_ENABLED
CFLAGS = $(USER_DEFINES) -std=gnu11 -Wall

#
# Object files
#
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(dir $(C_SOURCES))

all: build test

build: $(TARGET)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) -c $(CFLAGS) $(C_INCLUDES) -o $@ $<
	@echo "Building $*.c ..."

$(TARGET): $(OBJS)
	@$(CC) $(OBJS) -o $@
	@echo "Building $(patsubst $(PROJ_DIR)/%,%,$@) ..."
	$(SZ) $@
	@echo " "

test:
	behave .

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test
