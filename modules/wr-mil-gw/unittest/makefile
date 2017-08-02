CFLAGS  += -I..                                 \
		   -I../../../ip_cores/wrpc-sw/include/ \
		   -I../../../modules/lm32-include/     \
		   -DUNITTEST                           \
		   -g


		   
#LDFLAGS += 

VPATH=..

OBJECTS = wr_mil_utils.o     \
		  main.o             \

all: unittest
	./unittest

unittest: $(OBJECTS)
	gcc -o unittest $(OBJECTS)
