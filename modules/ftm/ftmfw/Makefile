TARGET    = ftm
BUILDPATH = ../../../syn/gsi_pexarria5/ftm

all: $(TARGET).bin

$(TARGET).%:
	make $@ -C $(BUILDPATH)

clean:
	rm -f $(BUILDPATH)/*.o $(BUILDPATH)/*.a $(BUILDPATH)/*.elf $(BUILDPATH)/*.bin

