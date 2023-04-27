# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# If using the Yocto SDK, you must additionally use YOCTO=YES
# Example usage:
#   'make YOCTO=YES DIM=YES PREFIX= all'                                      (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'               (hack: leave PREFIX empty for SCU path)

# install
PREFIX      ?= /usr/local
STAGING     ?=

# relative paths

# support Yocto SDK
ifeq ($(YOCTO), YES)
ARCH        := /x86_64
CFLAGS      ?=
USRPATH     := /common/usr/timing/b2b/yocto/usr
FTDI        := .
else
INCPATH      ?= .
ARCH        ?= /x86_64
USRPATH     ?= /opt/usr
CFLAGS      ?= -Wall -O2 -g
endif

ifeq ($(DIM), YES)
# DIM headers should be installed in USRPATH/include
# DIM library should be installed in USSPATH/lib
USEDIM      = -D USEDIM
DIMLIB      := -ldim
endif

EXTRA_FLAGS ?= $(USEDIM)
LIBS        ?= -L. -L$(USRPATH)/lib -Wl,-rpath,$(PREFIX)/lib -lm -lftd2xx -lmpsse $(DIMLIB) -lpthread
CCFLAGS     ?= $(EXTRA_FLAGS) -I../include -I$(USRPATH)/include 

$(info    CCFLAGS is $(CCFLAGS))
$(info     CFLAGS is $(CFLAGS))

TARGETS     := ftdimcp-ctl

all: $(TARGETS)

ftdimcp-ctl: ftdimcp-ctl.c
	$(CC) $(CFLAGS) $(CCFLAGS) -o ftdimcp-ctl ftdimcp-ctl.c ftdimcp-lib.c $(LIBS)

clean:
	rm -f *.o $(TARGETS)

deploy:

.PHONY: all clean
