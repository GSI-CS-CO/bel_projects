# Same program as idle-init, built for source-level debugging (no optimization, full debug info)
CFLAGS = -O0 -g3 -Wall -Wextra -fno-builtin

C_SOURCES += \
  main.c \
  $(NEORV32_COMMON_DIR)gsi_sdb_neorv32.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_uart.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_aux.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_cpu.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_rte.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_gpio.c

include ../common/neorv32.mk
