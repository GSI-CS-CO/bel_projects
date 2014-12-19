# OS/Embedded Target
BUILD = unix

# Target Options
ifeq ($(BUILD), unix)
FLAGS    = -fPIC -Wall -O2 -g
CFLAGS   = $(FLAGS) -Wmissing-declarations -Wmissing-prototypes -DPLATFORM=$(BUILD)
SOURCES  = main.c ../../src/c/nau8811_audio_driver.c
LIBS     = -Wl,-rpath,/usr/local/lib -letherbone
DEFINE   = -DNAU8811_TARGET_EMBEDDED=0
LIBRARY  = libetherbone.so
EXTRA    = libetherbone.so.*
endif

CFLAGS  := $(CFLAGS) -DPLATFORM=$(BUILD)
CXXFLAGS = $(FLAGS)
CC       = $(TARGET)gcc
CXX      = $(TARGET)g++

# Extended Warnings
WCFLAGS = -W -Wall -Wimplicit -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return \
          -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wmissing-declarations -Wuninitialized \
          -I../../src/c/

# Targets
all: eb-player

eb-player: $(SOURCES)
	$(CC) $(CFLAGS) $(WCFLAGS) $(DEFINE) -o $@ $^ $(LIBS)

clean:
	rm -f *.o *.elf *.bin eb-player
