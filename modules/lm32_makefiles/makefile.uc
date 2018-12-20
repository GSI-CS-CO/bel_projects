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
   QUIET = ""
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
   QUIET = @
endif

OPT_INCLUDE := $(addprefix -I,$(INCLUDE_DIRS) )
OPT_DEFINES := $(addprefix -D,$(DEFINES) )
ARG_LIBS    := $(addprefix -l,$(LIBS) )

OBJ_DIR ?= ./obj/
OBJ_FILES := $(addprefix $(OBJ_DIR),$(addsuffix .o,$(basename $(notdir $(SOURCE)))))

all: $(TARGET).bin size

DEPENDFILE = $(TARGET).dep

CC_ARG = $(CFLAGS) $(OPT_INCLUDE) $(OPT_DEFINES)
CXX_ARG ?= $(CC_ARG)
AS_ARG ?= $(CC_ARG)

$(OBJ_DIR):
	$(QUIET)mkdir $(OBJ_DIR)

$(DEPENDFILE): $(SOURCE) $(OBJ_DIR) $(ADDITIONAL_DEPENDENCES)
	$(QUIET)(for i in $(SOURCE); do \
		printf $(OBJ_DIR); \
		case "$${i##*.}" in \
		"cpp"|"CPP") \
			$(CXX) -MM $(CXX_ARG) "$$i"; \
			printf '\t$$(CXX_F) -c -o $$@ $$< $$(CXX_ARG)\n\n'; \
		;; \
		"c"|"C") \
			$(CC) -MM $(CC_ARG) "$$i"; \
			printf '\t$$(CC_F) -c -o $$@ $$< $$(CC_ARG)\n\n'; \
		;; \
		"s"|"S") \
			$(AS) -MM $(AS_ARG) "$$i"; \
			printf '\t$$(AS_F) -c -o $$@ $$< $$(AS_ARG)\n\n'; \
		;; \
		esac; \
	done) > $(DEPENDFILE)

.PHONY: dep
dep: $(DEPENDFILE)
	@cat $(DEPENDFILE)

-include $(DEPENDFILE)

$(TARGET).elf: $(OBJ_FILES) $(LINKER_SCRIPT)
	$(LD_F) -o $@ $^ $(LD_FLAGS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCPY_F) -O binary $< $@

.PHONY: size
size: $(TARGET).elf
	$(QUIET)$(SIZE) $(TARGET).elf

.PHONY: clean
clean:
	$(QUIET)rm $(ADDITIONAL_TO_CLEAN)
	$(QUIET)rm $(OBJ_DIR)*.*
	$(QUIET)rmdir $(OBJ_DIR)
	$(QUIET)rm $(DEPENDFILE)
	$(QUIET)rm $(TARGET).elf
	$(QUIET)rm $(TARGET).bin


#=================================== EOF ======================================