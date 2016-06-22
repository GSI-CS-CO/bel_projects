EBMPATH   = ../../../ip_cores/etherbone-core/hdl/eb_master_core
PRIOPATH  = ../../../modules/prioq2
XMLPATH   = /usr/include/libxml2
FLAGS   = -fPIC -Wall -O2 -g -I$(PRIOPATH) -I$(EBMPATH) -I$(XMLPATH)
CFLAGS  = $(FLAGS) -Wmissing-declarations -Wmissing-prototypes -DPLATFORM=$(BUILD) -std=gnu99
SOURCES	= ftmx86.c xmlaux.c access.c
LIBS    = -Wl,-rpath,/usr/local/lib -letherbone -lxml2
LIBRARY = libetherbone.so libxml2.so
EXTRA   = libetherbone.so.*



CXXFLAGS= $(FLAGS)
CC      = $(TARGET)gcc
CXX     = $(TARGET)g++

all: ftm-ctl

libftm.a: $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) $(LIBS)
	ar rcsv $@ *.o 

ftm-ctl:	libftm.a 
	$(CC) $(CFLAGS) -o $@ main.c $^ $(LIBS)

install: ftm-ctl
	cp ./ftm-ctl /usr/local/bin

clean:
	rm -f *.o *.a *.elf *.bin ftm-ctl mockup-test
	        
mockup: ftmx86.c xmlaux.c mockup.c
	$(CC) $(CFLAGS) -DMOCKUP=1 -c $^ -Wl,-rpath, -lxml2
	ar rcsv libftmmockup.a *.o  

mockup-test:	mockup
	$(CC) $(CFLAGS) -DMOCKUP=1 -o $@ main.c libftmmockup.a -Wl,-rpath, -lxml2



