
.PHOMY: all
all:
	$(MAKE) -C ./lm32Make  all
	$(MAKE) -C ./linuxMake all

.PHOMY: clean
clean:
	$(MAKE) -C ./lm32Make  clean
	$(MAKE) -C ./linuxMake clean

.PHOMY: load
load:
	$(MAKE) -C ./lm32Make  load

.PHONY: lload
lload:
	$(MAKE) -C ./linuxMake load

.PHOMY: run
run:
	$(MAKE) -C ./linuxMake run

.PHONY: dbg
dbg:
	$(MAKE) -C ./linuxMake dbg