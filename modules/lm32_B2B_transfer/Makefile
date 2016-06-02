TARGET1		  = B2B_main_src
TARGET2		  = B2B_main_trg
DEVICE		  = EP2AGX125
FLASH		    = EPCS128
RAM_SIZE	  = 65536
SHARED_SIZE = 24K
USRCPUCLK   = 125000

MYPATH   = ./
TLU = $(MYPATH)/../../ip_cores/wr-cores/modules/wr_tlu/lib
CFLAGS    = -I$(PSCU)/include -I$(MYPATH)/time_counter -I$(TLU)

include /home/jbai/test/bel_projects_B2B/bel_projects/syn/build.mk

$(TARGET1).elf: $(MYPATH)/B2B_main_src.c
$(TARGET2).elf: $(MYPATH)/B2B_main_trg.c

clean::
	rm -f *.o *.elf *.bin

                
