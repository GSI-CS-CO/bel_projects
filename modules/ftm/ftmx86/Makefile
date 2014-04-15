FLAGS   = -fPIC -Wall -O2 -g -I/usr/include/libxml2
CFLAGS  = $(FLAGS) -Wmissing-declarations -Wmissing-prototypes -DPLATFORM=$(BUILD)
SOURCES	= main.c ftmx86.c #xmlaux.c
LIBS    = -Wl,-rpath,/usr/local/lib -letherbone #-lxml2
LIBRARY = libetherbone.so #libxml2.so
EXTRA   = libetherbone.so.*


CFLAGS  := $(CFLAGS) -DPLATFORM=$(BUILD)
CXXFLAGS= $(FLAGS)
CC      = $(TARGET)gcc
CXX     = $(TARGET)g++

all: ftmctrl 

ftmctrl:	$(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)



clean:
	        rm -f *.o *.elf *.bin ftmctrl



