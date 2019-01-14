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

ifndef SOURCE
  $(error No sources defined in variable SOURCE !)
endif

ifndef NO_COLORED
   ESC_FG_CYAN    := "\\e[36m"
   ESC_FG_MAGNETA := "\\e[35m"
   ESC_FG_RED     := "\\e[31m"
   ESC_FG_GREEN   := "\\e[32m"
   ESC_BOLD       := "\\e[1m"
   ESC_NORMAL     := "\\e[0m"
endif

ifeq ($(V), 1)
   CC_F      = $(CC)
   CXX_F     = $(CXX)
   CPP_F     = $(CPP)
   CXXCPP_F  = $(CXXCPP)
   LD_F      = $(LD)
   AS_F      = $(AS)
   AR_F      = $(AR)
   OBJCPY_F  = $(OBJCPY)
else
   CXX_MSG     := "CXX"
   CC_MSG      := "CC"
   CPP_MSG     := "CPP"
   LD_MSG      := "LD"
   LDSH_MSG    := "LD SH-LIB"
   AR_MSG      := "AR"
   AS_MSG      := "AS"
   OBJCPY_MSG  := "OBJCPY"
   STRIP_MSG   := "STRIP"
   FORMAT      := @printf "[ %s %s ]\t%s\n"
 ifdef NO_COLORED
   FORMAT_L    := $(FORMAT)
   FORMAT_R    := $(FORMAT)
 else
   FORMAT_L    := @printf "[ %s %s ]\t$(ESC_FG_MAGNETA)%s$(ESC_NORMAL)\n"
   FORMAT_R    := @printf "[ %s %s ]\t$(ESC_FG_CYAN)$(ESC_BOLD)%s$(ESC_NORMAL)\n"
 endif
   CC_F        = $(FORMAT)   $(CPU) $(CC_MSG)     $(@); $(CC)
   CXX_F       = $(FORMAT)   $(CPU) $(CXX_MSG)    $(@); $(CXX)
   CPP_F       = $(FORMAT)   $(CPU) $(CPP_MSG)    $(@); $(CPP)
   CXXCPP_F    = $(FORMAT)   $(CPU) $(CPP_MSG)    $(@); $(CXXCPP)
   LD_F        = $(FORMAT_L) $(CPU) $(LD_MSG)     $(@); $(LD)
   AS_F        = $(FORMAT)   $(CPU) $(AS_MSG)     $(@); $(AS)
   AR_F        = $(FORMAT_L) $(CPU) $(AR_MSG)     $(@); $(AR)
   OBJCPY_F    = $(FORMAT_R) $(CPU) $(OBJCPY_MSG) $(@); $(OBJCPY)
   QUIET       = @
endif

_SOURCE       = $(strip $(SOURCE))
_INCLUDE_DIRS = $(strip $(INCLUDE_DIRS))

OPT_INCLUDE := $(addprefix -I,$(_INCLUDE_DIRS) )
OPT_DEFINES := $(addprefix -D,$(DEFINES) )
ARG_LIBS    := $(addprefix -l,$(LIBS) )

DEPLOY_DIR  ?= ./deploy_$(CPU)
WORK_DIR    ?= $(DEPLOY_DIR)/work/
TARGET_DIR  ?= $(DEPLOY_DIR)/result/

OBJ_FILES   := $(addprefix $(WORK_DIR),$(addsuffix .o,$(basename $(notdir $(_SOURCE)))))
ELF_FILE    = $(WORK_DIR)$(TARGET).elf
BIN_FILE    = $(TARGET_DIR)$(TARGET).bin
DEPENDFILE  = $(WORK_DIR)$(TARGET).dep

CC_ARG = $(CFLAGS) $(OPT_INCLUDE) $(OPT_DEFINES)
CXX_ARG ?= $(CC_ARG)
AS_ARG ?= $(CC_ARG)

RESULT_FILE ?= $(BIN_FILE)

all: $(RESULT_FILE) size

$(WORK_DIR):
	$(QUIET)mkdir -p $(WORK_DIR)

$(TARGET_DIR):
	$(QUIET)mkdir -p $(TARGET_DIR)

# TODO: Following rule could be made a bit better...

$(DEPENDFILE): $(_SOURCE) $(WORK_DIR) $(ADDITIONAL_DEPENDENCES)
	$(QUIET)(for i in $(_SOURCE); do \
		printf $(WORK_DIR); \
		case "$${i##*.}" in \
		"cpp"|"CPP") \
			$(CXX) -MM $(CXX_ARG) "$$i"; \
			[ "$$?" != "0" ] && echo :;  # prevents a makefile syntax error \
			printf '\t$$(CXX_F) -c -o $$@ $$< $$(CXX_ARG)\n\n'; \
		;; \
		"c"|"C") \
			$(CC) -MM $(CC_ARG) "$$i"; \
			[ "$$?" != "0" ] && echo :;  # prevents a makefile syntax error \
			printf '\t$$(CC_F) -c -o $$@ $$< $$(CC_ARG)\n\n'; \
		;; \
		"s"|"S") \
			$(AS) -MM $(AS_ARG) "$$i"; \
			[ "$$?" != "0" ] && echo :;  # prevents a makefile syntax error \
			printf '\t$$(AS_F) -c -o $$@ $$< $$(AS_ARG)\n\n'; \
		;; \
		esac; \
	done) > $(DEPENDFILE);


.PHONY: dep
dep: $(DEPENDFILE)
	@cat $(DEPENDFILE)

-include $(DEPENDFILE)


$(ELF_FILE): $(WORK_DIR)$(LINKER_SCRIPT) $(OBJ_FILES)
	$(LD_F) -o $@ $(OBJ_FILES) $(LD_FLAGS)

$(BIN_FILE): $(ELF_FILE) $(TARGET_DIR)
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

.PHONY: clean
clean:
	$(QUIET)( rm $(ADDITIONAL_TO_CLEAN); \
	rm $(OBJ_FILES); \
	rm $(ELF_FILE); \
	rm $(DEPENDFILE); \
	rm $(BIN_FILE); \
	rmdir $(WORK_DIR); \
	rmdir $(TARGET_DIR); \
	rmdir $(DEPLOY_DIR); \
	rmdir $(GENERATED_DIR) ) 2>/dev/null



#=================================== EOF ======================================