TARGETS = eb-flash eb-info
EB      = ../ip_cores/etherbone-core/api

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	cp $(TARGETS) /usr/local/bin

%:	%.c
	gcc -Wall -O2 -I $(EB) -o $@ $< -L $(EB) -letherbone
