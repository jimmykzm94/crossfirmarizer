#
# Toolchain
#
PREFIX = avr-
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

CHIP = ATmega2560
PROJECT = crossfirmarizer_$(CHIP)

#
# Paths
#
BUILD_DIR = $(PROJ_DIR)/build
DRIVERS_DIR = $(PROJ_DIR)/Drivers

TARGET = $(BUILD_DIR)/$(PROJECT).hex
MAP = $(BUILD_DIR)/$(PROJECT).map

#
# Source files
#
SOURCES = $(wildcard $(PROJ_DIR)/Core/Src/*.c) \
		  $(wildcard $(PROJ_DIR)/Core/Src/$(CHIP)/*.c)

REL_SOURCES = $(patsubst $(PROJ_DIR)/%,%,$(SOURCES))

# Object files
OBJS = $(addprefix $(BUILD_DIR)/, $(REL_SOURCES:.c=.o))

#
# Include paths
#
C_INCLUDES = -I$(PROJ_DIR)/Core/Inc \
			 -I$(PROJ_DIR)/Core/Inc/$(CHIP)

#
# Compiler flags
#
USER_DEFINES = -DTARGET_ATMEGA2560 -DULTRASONIC_SENSOR_ENABLED
CFLAGS = -mmcu=atmega2560 -DF_CPU=16000000UL $(USER_DEFINES) -std=gnu11 -Wall -Os $(C_INCLUDES)
LDFLAGS = -mmcu=atmega2560 -Wl,-Map=$(MAP)

all: $(TARGET)

$(BUILD_DIR)/%.o: $(PROJ_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) -o $@ $<
	@echo "Building $*.c ..."

$(BUILD_DIR)/$(PROJECT).elf: $(OBJS)
	@mkdir -p $(dir $@)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@
	@echo "Building $(patsubst $(PROJ_DIR)/%,%,$@) ..."
	$(SZ) $@
	@echo " "

$(TARGET): $(BUILD_DIR)/$(PROJECT).elf
	@$(CP) -O ihex -R .eeprom $< $@
	@echo "Building $(patsubst $(PROJ_DIR)/%,%,$@) ... done."

upload:
	@if [ -z "$(PORT)" ]; then \
			echo "Error: PORT argument required. Usage: make upload PORT=/dev/ttyUSB0"; \
			exit 1; \
	fi
	avrdude -p m2560 -c wiring -P $(PORT) -b 115200 -D -U flash:w:$(TARGET):i

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(MAP)

.PHONY: all clean upload