###############################################################################
##                                                                           ##
##              Common include makefile for non-OS targets                   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    makefile.uc                                                      ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    17.12.2018                                                       ##
###############################################################################
ifndef OBJCPY
   $(error No objcopy defined in variable OBJCPY !)
endif

DOX_INPUT   += $(MAKEFILE_DIR)/makefile.uc

ELF_FILE    = $(WORK_DIR)/$(TARGET).elf
BIN_FILE    = $(TARGET_DIR)/$(TARGET).bin

include $(MAKEFILE_DIR)/makefile.base

$(ELF_FILE): $(OBJ_FILES) $(ADDITIONAL_LD_DEPENDENCES)
	$(LD_F) -o $@ $(OBJ_FILES) $(LD_FLAGS)

$(BIN_FILE): $(ELF_FILE) # $(TARGET_DIR)
	$(OBJCPY_F) -O $(OUTPUT_FORMAT) $(ELF_FILE) $(BIN_FILE)

.PHONY: size
size: $(ELF_FILE)
	$(QUIET)$(SIZE) $(ELF_FILE)
ifdef USABLE_MEM_SIZE
	$(QUIET)(appSize=$$($(SIZE) $(ELF_FILE) | tail -n1 | awk '{printf $$4}'); \
	size=$$(echo $$(($${appSize}+$(RESERVED_MEM_SIZE)))); \
	free=$$(($(USABLE_MEM_SIZE)-$${size}));\
	if [ "$${free}" -lt "0" ]; then \
		ec=$(ESC_FG_RED); \
	else \
		ec=$(ESC_NORMAL); \
	fi; \
	echo -e "$${ec}$${size} of $(USABLE_MEM_SIZE) bytes used, $${free} \
	bytes free"; \
	echo -e "$(ESC_BOLD)>> Memory usage: $$(echo $${size}*100/$(USABLE_MEM_SIZE) \
	| bc)% <<$(ESC_NORMAL)")
endif

.PHONY: strings
strings: $(BIN_FILE)
	$(QUIET)$(STRINGS) $(BIN_FILE)


#=================================== EOF ======================================