# User LM32 firmware buildid generation
# M. Kreider, 28.01.2015
################################################################
# don't touch anything below unless you know what you're doing #
################################################################
RAM_OFFS	= 0x10000000
BOOTL_SIZE	= 0x100
BUILDID_SIZE	= 0x400
BUILDID_START	= $(shell printf "0x%x" $(BOOTL_SIZE)) 
SHARED_START    = $(shell printf "0x%x" $$(( $(BUILDID_START) + $(BUILDID_SIZE) )) ) 
INTADR_OFFS     = $(shell printf "0x%x" $(RAM_OFFS))
PLATFORM       ?= ""


ifdef SHARED_SIZE
	SHARED = "$(SHARED_SIZE)"
else
	SHARED = "0"
endif

LDS := "OUTPUT_FORMAT(\"elf32-lm32\")\n"
LDS := "MEMORY { RAM (rwx) : ORIGIN = $(RAM_OFFS), LENGTH = $(RAM_SIZE) }\n"
LDS += "SECTIONS\n{\n"
LDS += ". = ORIGIN(RAM);\n"
LDS += "_fstack = . + LENGTH(RAM) - 4;\n"
LDS += ".boot			: { _fboot   = .; *(.boot); 			     _eboot   = .; } > RAM\n"
LDS += ".buildid ADDR(.boot)   + $(BOOTL_SIZE) : { _fbuildid = .; *(.buildid .buildid.*) _ebuildid = .; } > RAM\n"
LDS += ".shared ADDR(.buildid) + $(BUILDID_SIZE) : { _fshared = .; PROVIDE(_startshared = .);*(.shared .shared.*)	_eshared = .; } > RAM\n"	
LDS += ".text ADDR(.shared)    + $(SHARED) : { _ftext = .; *(.text .text.*)	_etext = .; } > RAM\n"
LDS += ".rodata	  : { _frodata = .; *(.rodata .rodata.*) _erodata = .; } > RAM\n"
LDS += ".data			: { _fdata   = .; *(.data .data.*)     _edata   = .; } > RAM\n"
LDS += ".bss			: { _fbss    = .; *(.bss .bss.*)       _ebss    = .; } > RAM = 0\n}\n"


PKG := "library ieee;\n"
PKG += "use ieee.std_logic_1164.all;\n"
PKG += "use ieee.numeric_std.all;\n\n"
PKG += "library work;\n"
PKG += "package ramsize_pkg is\n"
PKG += "  constant c_lm32_ramsizes : natural := $(RAM_SIZE);\n"
PKG += "end ramsize_pkg;\n"



VERSION  ?= "1.0.0"
CBR_DATE := `date +"%a %b %d %H:%M:%S %Z %Y"`
CBR_USR  := `git config user.name`
CBR_MAIL := `git config user.email`
CBR_HOST := `hostname`
CBR_GCC  := `lm32-elf-gcc --version | grep gcc`
CBR_FLGS := $(CFLAGS)
CBR_KRNL := `uname -mrs`
CBR_OS   := `lsb_release -d -s | tr -d '"'` 
CBR_PF   := $(PLATFORM)
CBR_GIT1  := `git log HEAD~0 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT2  := `git log HEAD~1 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT3  := `git log HEAD~2 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT4  := `git log HEAD~3 --oneline --decorate=no -n 1 | cut -c1-100`
CBR_GIT5  := `git log HEAD~4 --oneline --decorate=no -n 1 | cut -c1-100`

CBR = "\#define BUILDID __attribute__((section(\".buildid\")))\nconst char BUILDID build_id_rom[] = \""'\\'"\nUserLM32"'\\n\\'"\nProject     : $(TARGET)"'\\n\\'"\nVersion     : $(VERSION)"'\\n\\'"\nPlatform    : $(CBR_PF)"'\\n\\'"\nBuild Date  : $(CBR_DATE)"'\\n\\'"\nPrepared by : $(USER) $(CBR_USR) <$(CBR_MAIL)>"'\\n\\'"\nPrepared on : $(CBR_HOST)"'\\n\\'"\nOS Version  : $(CBR_OS) $(CBR_KRNL)"'\\n\\'"\nGCC Version : $(CBR_GCC)"'\\n\\'"\nIntAdrOffs  : $(INTADR_OFFS)"'\\n\\'"\nSharedOffs  : $(SHARED_START)"'\\n\\'"\nSharedSize  : $(SHARED_SIZE)"'\\n\\'"\nFW-ID ROM will contain:"'\\n\\n\\'"\n   $(CBR_GIT1)"'\\n\\'"\n   $(CBR_GIT2)"'\\n\\'"\n   $(CBR_GIT3)"'\\n\\'"\n   $(CBR_GIT4)"'\\n\\'"\n   $(CBR_GIT5)"'\\n\\'"\n\";\n"

UCTARGET = `echo $(TARGET) | tr a-z A-Z`
SMM = "\#ifndef $(UCTARGET)_SHARED_MMAP_H\n\#define $(UCTARGET)_SHARED_MMAP_H\n//Location of Buildid and Shared Section in LM32 Memory, to be used by host\n\n\#define INT_BASE_ADR $(RAM_OFFS)\n\#define RAM_SIZE $(RAM_SIZE)\n\#define SHARED_SIZE $(SHARED_SIZE)\n\#define BUILDID_OFFS $(BUILDID_START)\n\#define SHARED_OFFS  $(SHARED_START)\n\#endif\n"
