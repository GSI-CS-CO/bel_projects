TARGET		=  scu_control

QUARTUS		?= /opt/quartus
QUARTUS_BIN	=  $(QUARTUS)/bin

all:	$(TARGET).jam

clean:
	rm -rf db incremental_db PLLJ_PLLSPE_INFO.txt
	rm -f $(TARGET).*.rpt $(TARGET).*.summary $(TARGET).map* $(TARGET).fit.* $(TARGET).pin $(TARGET).jdi $(TARGET)*.qdf
	rm -f $(TARGET).jam $(TARGET).jic $(TARGET).pof $(TARGET).sof $(TARGET).dep

%.sof:	%.qsf
	hdlmake
	$(QUARTUS_BIN)/quartus_map $*
	$(QUARTUS_BIN)/quartus_fit $*
	$(QUARTUS_BIN)/quartus_asm $*
	$(QUARTUS_BIN)/quartus_sta $*

%.dep:	%.qsf
	hdlmake
	echo -n "$*.sof $@:	$< " > $@.new
	echo `grep "set_global_assignment -name [^ ]*_FILE" $< | sed 's/^.*-name .*_FILE//;s/"//g'` >> $@.new
	mv $@.new $@

%.jic:	%.cof %.sof
	$(QUARTUS_BIN)/quartus_cpf -c $<

%.jam:	%.jic
	$(QUARTUS_BIN)/quartus_cpf -c $< $@

-include $(TARGET).dep
