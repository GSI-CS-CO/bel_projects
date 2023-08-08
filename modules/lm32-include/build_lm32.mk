# User LM32 firmware buildid generation
# M. Kreider, 28.01.2015 (reshaped by ARub, May 2018)
################################################################
# don't touch anything below unless you know what you're doing #
################################################################
CONFIG_RAMADDR	= 0x10000000
# RAM_SIZE below provided by project Makefile
CONFIG_RAMSIZE	= $(RAM_SIZE)
# Warning: these 0x100 and 0x400 are separately defined in some x86 makefile.
CONFIG_BOOTSIZE	= 0x100
CONFIG_BUILDIDSIZE = 0x400
# SHARED_SIZE below is optionally provided by project Makefile
SHARED_SIZE ?= 0
THR_QTY ?= 1
CONFIG_SHAREDSIZE = $(SHARED_SIZE)
# stack size is used to check for stack overflows, project Makefile cna override it
CONFIG_STACKSIZE ?= 10240

CONFIG_TARGET = $(shell echo $(TARGET) | tr a-z A-Z)

APPLY_CONFIG 	=  -e 's/CONFIG_TARGET/$(CONFIG_TARGET)/'
APPLY_CONFIG 	+= -e 's/CONFIG_RAMADDR/$(CONFIG_RAMADDR)/'
APPLY_CONFIG 	+= -e 's/CONFIG_RAMSIZE/$(CONFIG_RAMSIZE)/'
APPLY_CONFIG 	+= -e 's/CONFIG_STACKSIZE/$(CONFIG_STACKSIZE)/'
APPLY_CONFIG 	+= -e 's/CONFIG_BOOTSIZE/$(CONFIG_BOOTSIZE)/'
APPLY_CONFIG 	+= -e 's/CONFIG_BUILDIDSIZE/$(CONFIG_BUILDIDSIZE)/'
APPLY_CONFIG 	+= -e 's/CONFIG_SHAREDSIZE/$(CONFIG_SHAREDSIZE)/'

SMM = "\#ifndef $(UCTARGET)_SHARED_MMAP_H\n\#define $(UCTARGET)_SHARED_MMAP_H\n//Location of Buildid and Shared Section in LM32 Memory, to be used by host\n\n\#define INT_BASE_ADR $(CONFIG_RAMADDR)\n\#define RAM_SIZE $(CONFIG_RAMSIZE)\n\#define SHARED_SIZE $(SHARED_SIZE)\n\#define BUILDID_OFFS $(BUILDID_START)\n\#define SHARED_OFFS  $(SHARED_START)\n\#endif\n"

BUILDID_START	= $(shell printf "0x%x" $(CONFIG_BOOTSIZE)) 
SHARED_START    = $(shell printf "0x%x" $$(( $(BUILDID_START) + $(CONFIG_BUILDIDSIZE))))
PLATFORM       ?= ""

PKG := "library ieee;\n"
PKG += "use ieee.std_logic_1164.all;\n"
PKG += "use ieee.numeric_std.all;\n\n"
PKG += "library work;\n"
PKG += "package ramsize_pkg is\n"
PKG += "  constant c_lm32_ramsizes : natural := $(CONFIG_RAMSIZE);\n"
PKG += "end ramsize_pkg;\n"

GCC_PATH  = $(shell which $(CC))
GCC_BUILD = $(shell strings $(GCC_PATH) /dev/null | grep /share/locale | grep -oP 'lm32-gcc-\K.*' | cut -d'/' -f1)
GCC_VER   := `$(CC) --version | grep gcc`
CBR_GCC   = "$(GCC_VER) " (build " $(GCC_BUILD)")""

VERSION  ?= "1.0.0"
CBR_DATE := `date +"%a %b %d %H:%M:%S %Z %Y"`
CBR_USR  := `git config user.name`
CBR_MAIL := `git config user.email`
CBR_HOST := `hostname`
CBR_FLGS := $(CFLAGS)
CBR_KRNL := `uname -mrs`
CBR_OS   := `lsb_release -d -s | tr -d '"'` 
CBR_PF   := $(PLATFORM)
CBR_GIT1  := `git log HEAD~0 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT2  := `git log HEAD~1 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT3  := `git log HEAD~2 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT4  := `git log HEAD~3 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT5  := `git log HEAD~4 --oneline --decorate=no -n 1 | cut -c1-100`

CBR = "\#define BUILDID __attribute__((section(\".buildid\")))\nconst char BUILDID build_id_rom[] = \""'\\'"\nUserLM32"'\\n\\'"\nStack Status:                                                       "'\\n\\'"\nProject     : $(TARGET)"'\\n\\'"\nVersion     : $(VERSION)"'\\n\\'"\nPlatform    : $(CBR_PF)"'\\n\\'"\nBuild Date  : $(CBR_DATE)"'\\n\\'"\nPrepared by : $(USER) $(CBR_USR) <$(CBR_MAIL)>"'\\n\\'"\nPrepared on : $(CBR_HOST)"'\\n\\'"\nOS Version  : $(CBR_OS) $(CBR_KRNL)"'\\n\\'"\nGCC Version : $(CBR_GCC)"'\\n\\'"\nIntAdrOffs  : $(CONFIG_RAMADDR)"'\\n\\'"\nSharedOffs  : $(SHARED_START)"'\\n\\'"\nSharedSize  : $(SHARED_SIZE)"'\\n\\'"\nThreadQty   : $(THR_QTY)"'\\n\\'"\nFW-ID ROM will contain:"'\\n\\n\\'"\n   $(CBR_GIT1)"'\\n\\'"\n   $(CBR_GIT2)"'\\n\\'"\n   $(CBR_GIT3)"'\\n\\'"\n   $(CBR_GIT4)"'\\n\\'"\n   $(CBR_GIT5)"'\\n\\'"\n\";\n"

