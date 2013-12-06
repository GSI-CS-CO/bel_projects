PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../ip_cores/etherbone-core/api
TARGETS := eb-flash eb-info

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB)
LIBS        ?= -L $(EB) -Wl,-rpath,$(PREFIX)/lib -letherbone

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)
