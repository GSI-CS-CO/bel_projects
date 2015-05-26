PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../ip_cores/etherbone-core/api
ECA	?= ../ip_cores/wr-cores/modules/wr_eca
TARGETS := eb-flash eb-info eb-console eb-config-nv eca-snoop

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(ECA)
LIBS        ?= -L $(EB) -L $(ECA) -Wl,-rpath,$(PREFIX)/lib -letherbone

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

%:	%.cpp
	g++ $(CFLAGS) -o $@ $< $(LIBS) -leca -ltlu
