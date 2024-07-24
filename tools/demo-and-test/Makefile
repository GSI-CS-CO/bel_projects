# PREFIX  controls where programs and libraries get installed
# Example usage:
#   make PREFIX=/usr all
@usage: make all
PWD     := $(shell pwd)
HOME    = $(PWD)/..
ODIR    = $(HOME)/demo-and-test
IDIR    = $(HOME)/demo-and-test
WBDIR   = $(HOME)
PREFIX  ?= /usr/local
EB      ?= ../../ip_cores/etherbone-core/api
INCLUDE = -I$(IDIR) -I$(WBDIR) -I$(PREFIX)/include -I$(EB)
LIB     = -L $(EB)/.libs -letherbone


all: 	eb_native_demo eb_perf_demo eb_tlu_demo eb_read_time eb_diff_time

eb_native_demo: $(IDIR)/eb_native_demo.c
	@echo Making eb_native_demo
	gcc -g -Wall $(INCLUDE) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_native_demo $(IDIR)/eb_native_demo.c $(LIB)

eb_read_time: $(IDIR)/eb_read_time.c
	@echo Making eb_read_time
	gcc -g -Wall $(INCLUDE) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_read_time $(IDIR)/eb_read_time.c $(LIB)

eb_diff_time: $(IDIR)/eb_diff_time.c
	@echo Making eb_diff_time
	gcc -g -Wall $(INCLUDE) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_diff_time $(IDIR)/eb_diff_time.c $(LIB)


eb_tlu_demo: $(IDIR)/eb_tlu_demo.c
	@echo Making eb_tlu_demo
	gcc -g -Wall $(INCLUDE) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_tlu_demo $(IDIR)/eb_tlu_demo.c $(LIB)

eb_perf_demo: $(IDIR)/eb_perf_demo.c
	@echo Making eb_perf_demo
	$(CC) -Wall $(INCLUDE) -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib -o $(ODIR)/eb_perf_demo $(IDIR)/eb_perf_demo.c $(LIB)

clean:
	@echo cleaning stuff
	rm -f eb_perf_demo
	rm -f eb_native_demo
	rm -f eb_tlu_demo
	rm -f eb_read_time
	rm -f eb_diff_time


