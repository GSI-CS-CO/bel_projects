#BUILD = lm32
#BUILD = win32
LOC    ?=
BUILD  = linux
PREFIX ?= /usr/local

# =================================================================================================
# BEGIN OF ASL DEPLOYMENT
# =================================================================================================
ifeq ($(LOC), ASL)
PREFIX  ?= /usr/local
STAGING ?=
EB      ?= ../../ip_cores/etherbone-core/api
WBDIR   ?= .
TARGETS := simple-display
SOURCES  = main-x86.c disp.c disp-oled.c disp-lcd.c ssd1325_serial_driver.c

DEFINE   = -DSSD1325_TARGET_EMBEDDED=0
EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -static-libstdc++
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm

all: simple-display

simple-display: $(SOURCES)
	$(CC) $(CFLAGS) $(WCFLAGS) $(DEFINE) -o $@ $^ $(LIBS) -I../../modules/oled_display

install:
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

clean:
	rm -f $(TARGETS)

# =================================================================================================
# END OF ASL DEPLOYMENT
# =================================================================================================

else
ifeq ($(BUILD), lm32)
TARGET  = lm32-elf-
SOURCES = main-lm32.c disp.c disp-oled.c disp-lcd.c
FLAGS   = -Wall -Os
CFLAGS  = $(FLAGS) -mmultiply-enabled -mbarrel-shift-enabled  -ggdb -ffreestanding
DEFINE  = -DSSD1325_TARGET_EMBEDDED=
endif

ifeq ($(BUILD), linux)
FLAGS   = -Wall -O2 -g -I../../modules/oled_display
CFLAGS  = $(FLAGS) -fcommon -Wmissing-declarations -Wmissing-prototypes -DPLATFORM=$(BUILD)
SOURCES = main-x86.c disp.c disp-oled.c disp-lcd.c ssd1325_serial_driver.c
LIBS    = -Wl,-rpath,$(PREFIX)/lib -letherbone
DEFINE  = -DSSD1325_TARGET_EMBEDDED=0
LIBRARY = libetherbone.so
EXTRA   = libetherbone.so.*
endif

CFLAGS  := $(CFLAGS) -DPLATFORM=$(BUILD)
CXXFLAGS= $(FLAGS)
CC      = $(TARGET)gcc
CXX     = $(TARGET)g++

# Extended Warnings
WCFLAGS = -W -Wall -Wimplicit -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return \
          -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wmissing-declarations -Wuninitialized

all: simple-display

simple-display: $(SOURCES)
	$(CC) $(CFLAGS) $(WCFLAGS) $(DEFINE) -o $@ $^ $(LIBS)

simple-display.bin: simple-display.elf
	lm32-elf-objcopy -O binary $< $@

simple-display.elf: crt0.o simple-display.o main-lm32.o
	$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^

install:
	cp simple-display $(STAGING)$(PREFIX)/bin

clean:
	rm -f *.o *.elf *.bin simple-display

endif
