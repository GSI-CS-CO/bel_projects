TARGET		= scu_control
DEVICE		= EP2AGX125
FLASH		= EPCS128

QUARTUS		?= /opt/quartus
QUARTUS_BIN	=  $(QUARTUS)/bin

all:	$(TARGET).sof $(TARGET).jic $(TARGET).rpd

clean:
	rm -rf db incremental_db PLLJ_PLLSPE_INFO.txt
	rm -f $(TARGET).*.rpt $(TARGET).*.summary $(TARGET).map* $(TARGET).fit.* $(TARGET).pin $(TARGET).jdi $(TARGET)*.qdf $(TARGET).done $(TARGET).qws
	rm -f $(TARGET).rpd $(TARGET).jic $(TARGET).pof $(TARGET).sof $(TARGET).dep

%.sof:	%.qsf
	$(QUARTUS_BIN)/quartus_map $*
	$(QUARTUS_BIN)/quartus_fit $*
	$(QUARTUS_BIN)/quartus_asm $*
	$(QUARTUS_BIN)/quartus_sta $*

%.dep:	%.qsf
	hdlmake
	echo -n "$*.sof $@:	$< " > $@.new
	echo `grep "set_global_assignment -name [^ ]*_FILE" $< | sed 's/^.*-name .*_FILE//;s/"//g'` >> $@.new
	mv $@.new $@

%.opt:	%.sof
	echo "BITSTREAM_COMPRESSION=ON" > $@

%.jic:	%.sof %.opt
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt -d $(FLASH) -s $(DEVICE) $< $@

%.pof:	%.sof %.opt
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt -d $(FLASH) $< $@

%.rpd:	%.pof
	$(QUARTUS_BIN)/quartus_cpf -c -o $*.opt $< $@

-include $(TARGET).dep
