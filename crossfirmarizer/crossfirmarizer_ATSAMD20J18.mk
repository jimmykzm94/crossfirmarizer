################################################################################
# Makefile for ATSAMD20J18 project
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

CHIP = ATSAMD20J18
VERSION_MAJOR := $(shell grep "VERSION_MAJOR" $(PROJ_DIR)/Core/Inc/version.h | awk '{print $$3}' | tr -d '\r"')
VERSION_MINOR := $(shell grep "VERSION_MINOR" $(PROJ_DIR)/Core/Inc/version.h | awk '{print $$3}' | tr -d '\r"')
VERSION_BUILD := $(shell grep "VERSION_BUILD" $(PROJ_DIR)/Core/Inc/version.h | awk '{print $$3}' | tr -d '\r"')
PROJECT = crossfirmarizer_FW_$(CHIP)_v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_BUILD)

#
# Paths
#
BUILD_DIR = $(PROJ_DIR)/build
CORE_DIR = $(PROJ_DIR)/Core
SAMD20_DIR = $(PROJ_DIR)/Microchip/SAMD20_DFP/3.7.231/samd20
CMSIS_DIR = $(PROJ_DIR)/Microchip/CMSIS

TARGET_ELF = $(BUILD_DIR)/$(PROJECT).elf
TARGET_HEX = $(BUILD_DIR)/$(PROJECT).hex
TARGET_LIST = $(BUILD_DIR)/$(PROJECT).list
MAP = $(BUILD_DIR)/$(PROJECT).map

#
# Source files
#
C_SOURCES = $(wildcard $(CORE_DIR)/Src/*.c) \
		    $(wildcard $(CORE_DIR)/Src/$(CHIP)/*.c) \
			$(SAMD20_DIR)/source/startup_samd20j18.c \
			$(SAMD20_DIR)/source/system_samd20j18.c

#
# Include paths
#
C_INCLUDES = -I$(CORE_DIR)/Inc \
			 -I$(CORE_DIR)/Inc/$(CHIP) \
			 -I$(SAMD20_DIR)/include \
			 -I$(CMSIS_DIR)/Include \

#
# Compiler flags
#
USER_DEFINES = -DTARGET_$(CHIP) -DULTRASONIC_SENSOR_ENABLED
MCU = -mcpu=cortex-m0plus -mthumb
DEFS = -D__SAMD20J18__ -DUSE_CMSIS_INIT
CFLAGS = $(MCU) $(DEFS) $(USER_DEFINES) $(C_INCLUDES) -std=gnu11 -Os -Wall --specs=nano.specs -ffunction-sections -fdata-sections
CFLAGS += -Wextra -Wno-unused-variable

#
# Linker flags
#
LDSCRIPT = $(SAMD20_DIR)/linker_script/samd20j18_flash.ld
LDFLAGS = $(MCU) -T$(LDSCRIPT) --specs=nosys.specs -Wl,-Map=$(MAP) -Wl,--gc-sections -static --specs=nano.specs
LDFLAGS += -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

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

upload: $(TARGET_HEX)
	pyocd flash -t atsamd20j18 $^

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean upload
