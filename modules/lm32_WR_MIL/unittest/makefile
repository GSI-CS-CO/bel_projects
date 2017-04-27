CFLAGS  += -I..                                 \
		   -I../../../ip_cores/wrpc-sw/include/ \
		   -I../../../modules/lm32-include/     \

		   
#LDFLAGS += 

VPATH=..

OBJECTS = wr_mil_piggy.o     \
		  wr_mil_eca_queue.o \
		  wr_mil_eca_ctrl.o  \
		  main.o             \

unittest: $(OBJECTS)
	gcc -o unittest $(OBJECTS)
