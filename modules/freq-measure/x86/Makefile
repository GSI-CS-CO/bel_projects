# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Note: setting the PKG_CONFIG_PATH might help too, example:
# 'export PKG_CONFIG_PATH=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64/lib/pkgconfig/'
# Example usage:
#   'make clean'                                                 
#   'make PREFIX= all' ;                                                               (hack: leave PREFIX empty for SCU path)
# Example deploy:
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b-dev deploy'                    (hack: leave PREFIX empty for SCU path)
#   'make PREFIX= STAGING=/common/export/timing-rte/b2b deploy'                        (hack: leave PREFIX empty for SCU path)
PREFIX      ?= /usr/local
STAGING     ?=
TARGETS     := freq-measure

#DIMPATH     := /opt/dim/dim_v20r29
DIMPATH     := /common/usr/timing/b2b/dim_v20r29

#RPATH       ?= $(shell pkg-config saftlib --libs | sed 's/-l.*/ /g' | sed 's/-L//g')

#CXXFLAGS = `pkg-config saftlib --cflags` -I $(DIMPATH)/dim
#LDFLAGS  = `pkg-config saftlib --libs`   -L $(DIMPATH)/linux -ldim -lpthread -Wl,-rpath=$(RPATH)
#CXX = g++ --std=c++11

ASLLIB      := /common/export/timing-rte/tg-fallout-v6.2.0/x86_64/lib
ASLINC      := /common/export/timing-rte/tg-fallout-v6.2.0/x86_64/include/saftlib

CFLAGS      ?= `pkg-config  saftlib --cflags` $(EXTRA_FLAGS) -Wall -O2 -g  -I $(ASLINC) -I $(DIMPATH)/dim
LIBS        ?= `pkg-config  saftlib --libs` -L $(ASLLIB) -L $(DIMPATH)/linux -Wl,-rpath,$(PREFIX)/lib -lm 


all: $(TARGETS)

freq-measure: freq-measure.cpp
	g++ --std=c++11 $(CFLAGS) -o freq-measure freq-measure.cpp $(LIBS) -ldim -lpthread

clean: 
	rm -f freq-measure
