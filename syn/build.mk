TOP		:= $(dir $(lastword $(MAKEFILE_LIST)))..
QUARTUS		?= /opt/quartus
QUARTUS_BIN	=  $(QUARTUS)/bin
HDLMAKE := PATH=$(TOP)/bin:$(QUARTUS_BIN):$(PATH) PYTHONPATH=$(TOP)/lib/python2.7/site-packages hdlmake
PATHPKG		?= $(shell $(HDLMAKE) list-mods | grep -G '^[^\#]' | grep top | grep -o '^\S*')
SPI_LANES	?= ASx1

CROSS_COMPILE	?= lm32-elf-
CC		=  $(CROSS_COMPILE)gcc
SHELL = /bin/sh
OBJCOPY		=  $(CROSS_COMPILE)objcopy
GENRAMMIF	?= $(TOP)/ip_cores/wrpc-sw/tools/genrammif
INCPATH	:= $(TOP)/modules/lm32-include
EBPATH  := $(TOP)/ip_cores/etherbone-core/hdl/eb_master_core
W1    	:= $(TOP)/ip_cores/wrpc-sw
USRCPUCLK	?= 62500
CFLAGS	+= 	-mmultiply-enabled -mbarrel-shift-enabled -Os -DUSRCPUCLK=$(USRCPUCLK) -I$(INCPATH) -I$(W1)/include \
		-I$(W1)/sdb-lib -I$(W1)/pp_printf -I$(EBPATH) -std=gnu99 -DCONFIG_WR_NODE -DCONFIG_PRINT_BUFSIZE=128 -DSDBFS_BIG_ENDIAN


STUBD	?= $(TOP)/modules/lm32_stub
STUBS	?= $(STUBD)/stubs.c $(STUBD)/crt0.S
INCLUDES  += 	$(INCPATH)/dbg.c $(INCPATH)/aux.c $(INCPATH)/irq.c $(INCPATH)/mini_sdb.c $(INCPATH)/mprintf.c \
		$(W1)/dev/uart.c $(W1)/lib/usleep.c $(W1)/dev/devicelist.c $(W1)/dev/syscon.c $(W1)/pp_printf/printf.c \
		$(W1)/sdb-lib/glue.c $(W1)/pp_printf/vsprintf-mini.c $(INCPATH)/sdb_add.S
LDFLAGS		?= -nostdlib -T ram.ld -Wl,--defsym,_fstack=$$(($(RAM_SIZE)-4)) -lgcc -lc

ifndef RAM_SIZE
$(error Missing mandatory RAM_SIZE parameter! Quitting ...)
endif

.PHONY: ram.ld buildid.c $(PATHPKG)/ramsize_pkg.vhd $(TARGET)_shared_mmap.h
.PRECIOUS: $(TARGET).bin

include $(INCPATH)/build_lm32.mk
 

all:	$(TARGET).mif $(TARGET)_stub.mif $(TARGET).sof $(TARGET).jic $(TARGET).rpd

$(TARGET)_shared_mmap.h:
	@(printf %b $(SMM)) > $@

buildid.c:
	@(printf %b $(CBR)) > $@

ram.ld:
	@(printf %b $(LDS)) > $@

$(PATHPKG)/ramsize_pkg.vhd:
	@(printf %b $(PKG)) > $@

clean::
	rm -rf db incremental_db PLLJ_PLLSPE_INFO.txt
	rm -f $(TARGET).*.rpt $(TARGET).*.summary $(TARGET).map* $(TARGET).fit.* $(TARGET).pin $(TARGET).jdi $(TARGET)*.qdf $(TARGET).done $(TARGET).qws
	rm -f $(TARGET).rpd $(TARGET).jic $(TARGET).pof $(TARGET).sof $(TARGET).dep $(TARGET).elf $(TARGET).o *.mif *.elf
	rm -f ram.ld buildid.c $(TARGET)_shared_mmap.h 
	rm -f project project.tcl files.tcl

prog:
	@read -p "If you have multiple USB-Programmer connected, choose the one you want to use: " BLASTER; \
	[ -z "$$BLASTER" ] && BLASTER=1 ; \
	$(QUARTUS_BIN)/quartus_pgm -c "$$BLASTER" -m jtag -o 'p;$(TARGET).sof'

%_stub.elf:  ram.ld
	$(CC) $(CFLAGS) -o $@ $^ $(STUBS) $(LDFLAGS)

%.elf:	buildid.c ram.ld $(TARGET)_shared_mmap.h
	$(CC) $(CFLAGS) -o $@ $^ $(STUBS) $(INCLUDES) $(LDFLAGS)

%.bin:	%.elf
	$(OBJCOPY) -O binary $< $@

%.mif:	%.bin
	$(GENRAMMIF) $< $(RAM_SIZE) > $@

%.sof:	%.qsf %.mif $(PATHPKG)/ramsize_pkg.vhd 
	$(HDLMAKE) makefile -f hdlmake.mk ; make -f hdlmake.mk project
	find $(TOP) -name Manifest.py > $*.dep
	sed -n -e 's/"//g;s/quartus_sh://;s/set_global_assignment.*-name.*_FILE //p' < $< >> $*.dep
	echo "$*.sof $@:	$< " `cat $*.dep` > $*.dep
	$(QUARTUS_BIN)/quartus_sh --tcl_eval load_package flow \; project_open $* \; execute_flow -compile

%.opt:	%.sof
	rm -f $@.tmp
	[ $$($(QUARTUS_BIN)/quartus_cpf --version | sed -ne 's/^Version \([0-9]*\).*$$/\1/p') -lt 13 ] || \
		echo "IGNORE_EPCS_ID_CHECK=ON" >> $@.tmp
	echo "BITSTREAM_COMPRESSION=ON" >> $@.tmp
	mv $@.tmp $@

%.jic:	%.sof %.opt
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt -d $(FLASH) -s $(DEVICE) $< $@

%.pof:	%.sof %.opt
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt -d $(FLASH) -m $(SPI_LANES) $< $@

%.rpd:	%.pof %.opt
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt $< $@

-include $(TARGET).dep
