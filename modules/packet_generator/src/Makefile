PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../../../ip_cores/etherbone-core/api
TARGETS := pkg_gen_conf

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

%:	%.cpp
	g++ $(CFLAGS) -o $@ $< $(LIBS)
