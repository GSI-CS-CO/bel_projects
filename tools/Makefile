TARGETS = eb-flash

all:	$(TARGETS)
clean:
	rm -f $(TARGETS)
install:
	cp $(TARGETS) /usr/local/bin

%:	%.c
	gcc -Wall -O2 -o $@ $< -L ../ip_cores/etherbone-core/api -letherbone

