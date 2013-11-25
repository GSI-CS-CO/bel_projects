TARGET		= vetar
DEVICE		= EP2AGX125
FLASH		= EPCS128

QUARTUS		?= /opt/quartus
QUARTUS_BIN	=  $(QUARTUS)/bin

all:	$(TARGET).sof $(TARGET).jic $(TARGET).rpd

prog: 
	@read -p "If there are more than one USB-Programmer specify number you want to use: " BLASTER; \
	$(QUARTUS_BIN)/quartus_pgm -c "$$BLASTER" -m jtag -o 'p;$(TARGET).sof'

clean:
	rm -rf db incremental_db PLLJ_PLLSPE_INFO.txt
	rm -f $(TARGET).*.rpt $(TARGET).*.summary $(TARGET).map* $(TARGET).fit.* $(TARGET).pin $(TARGET).jdi $(TARGET)*.qdf $(TARGET).done $(TARGET).qws
	rm -f $(TARGET).rpd $(TARGET).jic $(TARGET).pof $(TARGET).sof $(TARGET).dep

%.sof:	%.qsf
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
