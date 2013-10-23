PREFIX  ?= /usr/local
EB      ?= ../ip_cores/etherbone-core/api
TARGETS := eb-flash eb-info

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	cp $(TARGETS) $(PREFIX)/bin

%:	%.c
	gcc -Wall -O2 -I $(EB) -o $@ $< -L $(EB) -Wl,-rpath,$(PREFIX)/lib -letherbone
