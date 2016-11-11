# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
PREFIX      ?= /usr/local
STAGING     ?=
EXTRA_FLAGS ?=
TARGETS := wr-mon

FLAGS   = -fPIC
LIBS    = -Wl,-rpath,$(PREFIX)/lib -L$(PREFIX)/lib -letherbone
INCLUDE = $(PREFIX)/include

FLAGS	:= $(FLAGS) -I$(INCLUDE) -Wall -O2
CFLAGS	= $(EXTRA_FLAGS) $(FLAGS) -Wmissing-declarations -Wmissing-prototypes
CC	= $(TARGET)gcc

# special for wr-mon
WBDIR    = ..
WBFLAG   = -DNOWB_SIMULATE
WBFLAG  += -I$(WBDIR) 

all: $(TARGETS)

wr-mon: wr-mon.c
	$(CC) $(CFLAGS) $(WBFLAG) $(WBDIR)/wb_api.c -o wr-mon wr-mon.c $(LIBS)

clean:
	rm -f *.o wr-mon

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

.PHONY: all clean
