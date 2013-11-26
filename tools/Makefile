PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../ip_cores/etherbone-core/api
TARGETS := eb-flash eb-info

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	gcc -Wall -O2 -I $(EB) -o $@ $< -L $(EB) -Wl,-rpath,$(PREFIX)/lib -letherbone
