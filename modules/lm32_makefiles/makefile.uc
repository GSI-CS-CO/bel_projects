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

ifeq ($(VERBOSE), 1)
   CC_F      = $(CC)
   CXX_F     = $(CXX)
   CPP_F     = $(CPP)
   CXXCPP_F  = $(CXXCPP)
   LD_F      = $(LD)
   AS_F      = $(AS)
   AR_F      = $(AR)
   OBJCPY_F = $(OBJCPY)
else
   CXX_MSG    := "CXX"
   CC_MSG     := "CC"
   CPP_MSG    := "CPP"
   LD_MSG     := "LD"
   LDSH_MSG   := "LD SH-LIB"
   AR_MSG     := "AR"
   AS_MSG     := "AS"
   OBJCPY_MSG := "OBJCOPY"
   STRIP_MSG  := "STRIP"
   FORMAT     := @printf "[ %s %s ]\t%s\n"
   CC_F      = $(FORMAT) $(CPU) $(CC_MSG)     $(@); $(CC)
   CXX_F     = $(FORMAT) $(CPU) $(CXX_MSG)    $(@); $(CXX)
   CPP_F     = $(FORMAT) $(CPU) $(CPP_MSG)    $(@); $(CPP)
   CXXCPP_F  = $(FORMAT) $(CPU) $(CPP_MSG)    $(@); $(CXXCPP)
   LD_F      = $(FORMAT) $(CPU) $(LD_MSG)     $(@); $(LD)
   AS_F      = $(FORMAT) $(CPU) $(AS_MSG)     $(@); $(AS)
   AR_F      = $(FORMAT) $(CPU) $(AR_MSG)     $(@); $(AR)
   OBJCPY_F  = $(FORMAT) $(CPU) $(OBJCPY_MSG) $(@); $(OBJCPY)
endif

OPT_INCLUDE := $(addprefix -I,$(INCLUDE_DIRS) )
OPT_DEFINES := $(addprefix -D,$(DEFINES) )
ARG_LIBS    := $(addprefix -l,$(LIBS) )


TARGET ?= $(notdir $(basename $(MIAN_MODULE)))
SOURCE += $(MIAN_MODULE)

OBJ_DIR ?= ./obj/
OBJ_FILES := $(addprefix $(OBJ_DIR),$(addsuffix .o,$(basename $(notdir $(SOURCE)))))

all: $(TARGET).bin

DEPENDFILE = $(TARGET).dep

CC_ARG = $(CFLAGS) $(OPT_INCLUDE) $(OPT_DEFINES)
CXX_ARG ?= $(CC_ARG)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

C_SOURCE = $(SOURCE) # TODO: Separate C++, C and Assembler files.
	
$(DEPENDFILE): $(C_SOURCE) $(CPP_SOURCE) $(OBJ_DIR)
	@(for i in $(C_SOURCE); do printf $(OBJ_DIR); $(CC) -MM $(CC_ARG) "$$i"; \
	printf '\t$$(CC_F) -c -o $$@ $$< $$(CC_ARG)\n\n'; done) > $(DEPENDFILE)
	@(for i in $(CXX_SOURCE); do printf $(OBJ_DIR); $(CXX) -MM $(CXX_ARG) "$$i"; \
	printf '\t$$(CXX_F) -c -o $$@ $$< $$(CXX_ARG)\n\n'; done) >> $(DEPENDFILE)

.PHONY: dep
dep: $(DEPENDFILE)
	@cat $(DEPENDFILE)

#$(TARGET).elf: $(OBJ_FILES)
#	$(LD_F) -o $@ $^ -Wl,--defsym,_fstack=$$(($(RAM_SIZE)-4)) -nostdlib -T ram.ld $(ARG_LIBS)

$(TARGET).elf: $(OBJ_FILES)
	$(CC_F) -o $@ $^ -Wl,--defsym,_fstack=$$(($(RAM_SIZE)-4)) -nostdlib -T ram.ld $(ARG_LIBS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCPY_F) -O binary $< $@

.PHONY: size
size: $(TARGET).elf
	$(SIZE) $(TARGET).elf

.PHONY: clean
clean:
	rm $(OBJ_DIR)*.*
	rmdir $(OBJ_DIR)
	rm $(DEPENDFILE)
	rm $(TARGET).elf
	rm $(TARGET).bin

-include $(DEPENDFILE)
#=================================== EOF ======================================