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
VERSION_MAJOR := $(shell grep "VERSION_MAJOR" $(PROJ_DIR)/Core/Inc/version.h | awk '{print $$3}' | tr -d '\r"')
VERSION_MINOR := $(shell grep "VERSION_MINOR" $(PROJ_DIR)/Core/Inc/version.h | awk '{print $$3}' | tr -d '\r"')
VERSION_BUILD := $(shell grep "VERSION_BUILD" $(PROJ_DIR)/Core/Inc/version.h | awk '{print $$3}' | tr -d '\r"')
PROJECT = crossfirmarizer_FW_$(CHIP)_v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD)

#
# Paths
#
BUILD_DIR = $(PROJ_DIR)/build
CORE_DIR = $(PROJ_DIR)/Core
DRIVERS_DIR = $(PROJ_DIR)/Drivers
CMSIS_DIR = $(DRIVERS_DIR)/CMSIS

TARGET_ELF = $(BUILD_DIR)/$(PROJECT).elf
TARGET_HEX = $(BUILD_DIR)/$(PROJECT).hex
TARGET_LIST = $(BUILD_DIR)/$(PROJECT).list
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
USER_DEFINES = -DTARGET_STM32F446RE -DULTRASONIC_SENSOR_ENABLED
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard 
DEFS = -DSTM32F446xx
CFLAGS = $(MCU) $(DEFS) $(USER_DEFINES) $(C_INCLUDES) -std=gnu11 -Os -Wall --specs=nano.specs -ffunction-sections -fdata-sections
# CFLAGS += -Wextra -Wno-unused-variable
ASMFLAGS = $(MCU) -x assembler-with-cpp --specs=nano.specs

#
# Linker flags
#
LDSCRIPT = $(PROJ_DIR)/STM32F446RETX_FLASH.ld
LDFLAGS = $(MCU) -T$(LDSCRIPT) --specs=nosys.specs -Wl,-Map=$(MAP) -Wl,--gc-sections -static --specs=nano.specs
LDFLAGS += -Wl,--start-group -lc -lm -Wl,--end-group

#
# Object files
#
OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(TARGET_ELF) $(TARGET_HEX) $(TARGET_LIST)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo "Building $*.c ..."

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(BUILD_DIR)
	@$(CC) -c $(ASMFLAGS) -o $@ $<
	@echo "Building $*.s ..."

$(TARGET_ELF): $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@
	@echo "Building $(patsubst $(PROJ_DIR)/%,%,$@) ..."
	$(SZ) $@
	@echo " "

$(TARGET_HEX): $(TARGET_ELF)
	@$(CP) -O ihex $< $@
	@echo "Building $(patsubst $(PROJ_DIR)/%,%,$@) ... done."

$(TARGET_LIST): $(TARGET_ELF)
	@$(OD) -h -S $< > $@
	@echo "Building $(patsubst $(PROJ_DIR)/%,%,$@) ... done."

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
