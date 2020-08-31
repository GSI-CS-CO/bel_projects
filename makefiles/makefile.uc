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

# Main functions in microcontrollers do not necessarily have
# to have a return value.
CFLAGS += -Wno-main

ELF_FILE    = $(WORK_DIR)/$(TARGET).elf
BIN_FILE    = $(TARGET_DIR)/$(TARGET).bin

include $(MAKEFILE_DIR)/makefile.base

$(ELF_FILE): $(OBJ_FILES) $(ADDITIONAL_LD_DEPENDENCES)
	$(LD_F) -o $@ $(OBJ_FILES) $(_LD_FLAGS)

$(BIN_FILE): $(ELF_FILE) # $(TARGET_DIR)
	$(OBJCPY_F) -O $(OUTPUT_FORMAT) $(ELF_FILE) $(BIN_FILE)

.PHONY: size
size: $(ELF_FILE)
	$(QUIET)$(SIZE) $(ELF_FILE)
ifdef USABLE_MEM_SIZE
	$(QUIET)(appSize=$$($(SIZE) $(ELF_FILE) | tail -n1 | awk '{printf $$4}'); \
	size=$$(echo $$(($${appSize}+$(RESERVED_MEM_SIZE)))); \
	free=$$(($(RAM_SIZE)-$${appSize}));\
	if [ "$${free}" -lt "0" ]; then \
		ec=$(ESC_FG_RED); \
	else \
		ec=$(ESC_FG_CYAN); \
	fi; \
	echo "Total RAM size:     $$(($(RAM_SIZE))) bytes";\
	echo "Stack size:         $$(($(STACK_SIZE))) bytes";\
	echo "Shared memory size: $$(($(SHARED_SIZE))) bytes";\
	echo "Build-ID size:      $$(($(BUILDID_SIZE))) bytes";\
	echo "Boot size:          $$(($(BOOTL_SIZE))) bytes";\
	echo -e "$${ec}Consumption:        $${appSize} of $(USABLE_MEM_SIZE) bytes used, $${free} \
	bytes free"; \
	echo -e "$(ESC_BOLD)-->> Memory consumption of application \"$(TARGET)\" \
	in $(CPU): $$(echo $${appSize}*100/$(USABLE_MEM_SIZE) \
	| bc)%, Optimization level:" $(CODE_OPTIMIZATION) "LTO:" $(LTO_INFO) "<<--$(ESC_NORMAL)" )
endif

.PHONY: strings
strings: $(BIN_FILE)
	$(QUIET)$(STRINGS) $(BIN_FILE)

.PHONY: objdump
objdump: $(ELF_FILE)
	$(OBJDUMP_F) -h $(ELF_FILE)

#=================================== EOF ======================================