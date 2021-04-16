# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   'make MASP=YES PRO=NO PREFIX= all'                                                  (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make MASP=YES PRO=NO PREFIX= STAGING=/common/export/timing-rte/XYZ deploy'         (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
ARCH        ?= x86_64
INCLUDE     ?=../include
TARGETS     := phtif

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I$(INCLUDE)  -I../../../modules/common-libs/include
LIBS        ?= -L . -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -lm 
#CFLAGS      ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g -I $(EB) -I$(INCLUDE) -I../../../modules/common-libs/include
#LIBS        ?= `pkg-config  saftlib --libs` -L . -L $(ASLLIB) -Wl,-rpath,$(PREFIX)/lib -lm 



all: $(TARGETS)


phtif: phtif.c
	$(CC) $(CFLAGS) -o phtif phtif.c ../../../modules/common-libs/x86/ivtpar.c $(LIBS)

clean:
	rm -f *.o phtif.o phtif

deploy:
	mkdir -p $(STAGING)/$(ARCH)$(PREFIX)/bin	
	cp $(TARGETS) $(STAGING)/$(ARCH)$(PREFIX)/bin	
#	cp $(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib
#	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1.0 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1
#	ln -sf $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB).1 $(STAGING)$(ARCH)$(PREFIX)/lib/$(B2BLIB)

.PHONY: all clean
