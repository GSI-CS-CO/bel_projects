FLAGS   = -fPIC -Wall -O2 -g -I/usr/include/libxml2
CFLAGS  = $(FLAGS) -Wmissing-declarations -Wmissing-prototypes -DPLATFORM=$(BUILD) -std=gnu99
SOURCES	= main.c ftmx86.c xmlaux.c access.c
LIBS    = -Wl,-rpath,/usr/local/lib -letherbone -lxml2
LIBRARY = libetherbone.so libxml2.so
EXTRA   = libetherbone.so.*


CFLAGS  := $(CFLAGS) -DPLATFORM=$(BUILD)
CXXFLAGS= $(FLAGS)
CC      = $(TARGET)gcc
CXX     = $(TARGET)g++

all: ftm-ctl

ftm-ctl:	$(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)



clean:
	        rm -f *.o *.elf *.bin ftm-ctl



