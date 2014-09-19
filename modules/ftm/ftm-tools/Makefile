DESTPATH = .
FWPATH   = ../ftmfw
TESTPATH = ../ftmverification
TOOLPATH = ../ftmx86
  
all:	tool firmware testfw
	#if test -d $(DESTPATH); \
     	#then echo exists; \
     	#else mkdir $(DESTPATH); \
     	#fi	
	cp $(TOOLPATH)/ftm-ctl $(DESTPATH) 
	cp $(TOOLPATH)/*.xml   $(DESTPATH)
	cp $(TOOLPATH)/*manual*.txt   $(DESTPATH)
	cp $(TESTPATH)/*.elf $(DESTPATH) 
	cp $(TESTPATH)/*.bin $(DESTPATH) 
	cp $(TESTPATH)/ecaconf $(DESTPATH) 
	cp $(FWPATH)/*.elf $(DESTPATH) 
	cp $(FWPATH)/*.bin $(DESTPATH)
	echo
	echo All files are ready in $(DESTPATH) 

tool:
	$(MAKE) -C $(TOOLPATH)
firmware:
	$(MAKE) -C $(FWPATH)
testfw:
	$(MAKE) -C $(TESTPATH)
         
clean:
	rm -f $(TOOLPATH)/*.o $(TOOLPATH)/*.elf $(TOOLPATH)/*.bin $(TOOLPATH)/ftm-ctl
	rm -f $(FWPATH)/*.o $(FWPATH)/*.elf $(FWPATH)/*.bin
	rm -f $(TESTPATH)/*.o $(TESTPATH)/*.elf $(TESTPATH)/*.bin
	rm -f $(DESTPATH)/*.o $(DESTPATH)/*.elf $(DESTPATH)/*.bin $(DESTPATH)/*.xml $(DESTPATH)/ftm-ctl
