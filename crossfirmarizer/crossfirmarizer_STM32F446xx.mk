################################################################################
# Makefile for STM32F446RE project
################################################################################

#
# Toolchain
#
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
CP = $(PREFIX)objcopy
OD = $(PREFIX)objdump
SZ = $(PREFIX)size

#
# Project settings
#
PROJ_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CHIP = STM32F446RE
PROJECT = crossfirmarizer_$(CHIP)

#
# Paths
#
BUILD_DIR = $(PROJ_DIR)/build
CORE_DIR = $(PROJ_DIR)/Core
DRIVERS_DIR = $(PROJ_DIR)/Drivers
CMSIS_DIR = $(DRIVERS_DIR)/CMSIS

TARGET = $(BUILD_DIR)/$(PROJECT).elf
MAP = $(BUILD_DIR)/$(PROJECT).map

#
# Source files
#
C_SOURCES = $(wildcard $(CORE_DIR)/Src/*.c) \
		    $(wildcard $(CORE_DIR)/Src/$(CHIP)/*.c)
ASM_SOURCES = $(wildcard $(CORE_DIR)/Startup/*.s)

#
# Include paths
#
C_INCLUDES = -I$(CORE_DIR)/Inc
C_INCLUDES += -I$(CMSIS_DIR)/Device/ST/STM32F4xx/Include
C_INCLUDES += -I$(CMSIS_DIR)/Include
C_INCLUDES += -I$(PROJ_DIR)/Core/Inc/$(CHIP)

#
# Compiler flags
#
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFS = -DSTM32F446xx
CFLAGS = $(MCU) $(DEFS) $(C_INCLUDES) -std=gnu11 -Os -Wall --specs=nano.specs -ffunction-sections -fdata-sections
CFLAGS += -Wextra -Wno-unused-variable
ASMFLAGS = $(MCU) -x assembler-with-cpp

#
# Linker flags
#
LDSCRIPT = STM32F446RETX_FLASH.ld
LIBS = -lc -lm
LDFLAGS = $(MCU) -T$(LDSCRIPT) --specs=nosys.specs -Wl,-Map=$(MAP) -Wl,--gc-sections -static --specs=nano.specs
LDFLAGS += -Wl,--start-group $(LIBS) -Wl,--end-group

#
# Object files
#
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(TARGET)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(ASMFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	@echo " "
	$(SZ) $(TARGET)
	@echo " "

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(MAP)

.PHONY: all clean
