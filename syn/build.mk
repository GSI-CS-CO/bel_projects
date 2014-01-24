QUARTUS		?= /opt/quartus
QUARTUS_BIN	=  $(QUARTUS)/bin

RAM_SIZE	?= 4096

CROSS_COMPILE	?= lm32-elf-
CC		=  $(CROSS_COMPILE)gcc
OBJCOPY		=  $(CROSS_COMPILE)objcopy
GENRAMMIF	?= ../../../ip_cores/wrpc-sw/tools/genrammif
CFLAGS		?= -mmultiply-enabled -mbarrel-shift-enabled -Os -DCPU_CLOCK=62500
STUBD		?= ../../../modules/lm32_stub
STUBS		?= $(STUBD)/stubs.c $(STUBD)/crt0.S
LDFLAGS		?= -nostdlib -T $(STUBD)/ram.ld -Wl,--defsym,_fstack=$(RAM_SIZE)-4 -lgcc -lc

all:	$(TARGET).mif $(TARGET).sof $(TARGET).jic $(TARGET).rpd

clean:
	rm -rf db incremental_db PLLJ_PLLSPE_INFO.txt
	rm -f $(TARGET).*.rpt $(TARGET).*.summary $(TARGET).map* $(TARGET).fit.* $(TARGET).pin $(TARGET).jdi $(TARGET)*.qdf $(TARGET).done $(TARGET).qws
	rm -f $(TARGET).rpd $(TARGET).jic $(TARGET).pof $(TARGET).sof $(TARGET).dep $(TARGET).elf $(TARGET).o

%.elf:
	$(CC) $(CFLAGS) -o $@ $^ $(STUBS) $(LDFLAGS)

%.bin:	%.elf
	$(OBJCOPY) -O binary $< $@

%.mif:	%.bin
	$(GENRAMMIF) $< $(RAM_SIZE) > $@

%.sof:	%.qsf %.mif
	hdlmake --quartus-proj -v | sed -n -e 's/ *$$/:/;s/^.* Parsing manifest file: *//p' > $*.dep
	sed -n -e 's/"//g;s/quartus_sh://;s/$$/:/;s/set_global_assignment.*-name.*_FILE //p' < $< >> $*.dep
	echo "$*.sof $@:	$< " `sed 's/ *: *$$//' < $*.dep` >> $*.dep
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
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt -d $(FLASH) $< $@

%.rpd:	%.pof %.opt
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt $< $@

-include $(TARGET).dep
