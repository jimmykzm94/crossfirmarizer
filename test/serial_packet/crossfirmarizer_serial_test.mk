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
BUILD_DIR = $(TEST_DIR)/build


# Build target for C
FW_DIR = $(PROJ_DIR)/../../crossfirmarizer/Core
SOURCE_C = $(FW_DIR)/Src/serial_packet.c ${TEST_DIR}/lang_c/main.c
TARGET_C = $(BUILD_DIR)/test_serial_packet.out
$(TARGET_C): ${SOURCE_C}
	@mkdir -p $(BUILD_DIR)
	@$(CC) -std=gnu11 -Wall -I$(FW_DIR)/Inc -o $@ $^

# Build target for Python
TARGET_PY = $(TEST_DIR)/lang_python/main.py

run_c: $(TARGET_C)
	@${TARGET_C} ${MODE} "${DATA}"

run_py:
	@python ${TARGET_PY} ${MODE} "${DATA}"

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: run_c run_py clean
