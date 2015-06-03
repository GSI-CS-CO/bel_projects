# User LM32 firmware buildid generation
# M. Kreider, 28.01.2015
################################################################
# don't touch anything below unless you know what you're doing #
################################################################
BOOTL_SIZE 	= 0x100
BUILDID_SIZE 	= 0x400

ifdef SHARED_SIZE
	SHARED = "$(BOOTL_SIZE) + $(BUILDID_SIZE) + $(SHARED_SIZE)"
else
	SHARED = ""
endif

LDS := "OUTPUT_FORMAT(\"elf32-lm32\")\n"
LDS += "SECTIONS\n{\n"
LDS += ".boot			: { _fboot    = .; *(.boot); 			_eboot      = .; }\n"
LDS += ".buildid $(BOOTL_SIZE)	: { _fbuildid = .; *(.buildid .buildid.*)   	_ebuildid   = .; }\n"
LDS += ".shared  $(BOOTL_SIZE) + $(BUILDID_SIZE)	: { _fshared  = .; PROVIDE(_startshared = .);*(.shared .shared.*) 	_eshared    = .; }\n"	
LDS += ".text    $(SHARED)	: { _ftext    = .; *(.text .text.*) 		_etext      = .; }\n"
LDS += ".rodata			: { _frodata  = .; *(.rodata .rodata.*)     	_erodata    = .; }\n"
LDS += ".data			: { _fdata    = .; *(.data .data.*)         	_edata      = .; }\n"
LDS += ".bss			: { _fbss     = .; *(.bss .bss.*)           	_ebss       = .; }\n}\n"

VERSION  ?= "1.0.0"
CBR_DATE := `date +"%a %b %d %H:%M:%S %Z %Y"`
CBR_USR  := `git config user.name`
CBR_MAIL := `git config user.email`
CBR_HOST := `hostname`
CBR_GCC  := `lm32-elf-gcc --version | grep gcc`
CBR_FLGS := $(CFLAGS)
CBR_KRNL := `uname -mrs`
CBR_OS   := `lsb_release -d -s` 
CBR_PF   := ""
CBR_GIT1  := `git log HEAD~0 --oneline --decorate=no -n 1`
CBR_GIT2  := `git log HEAD~1 --oneline --decorate=no -n 1`
CBR_GIT3  := `git log HEAD~2 --oneline --decorate=no -n 1`
CBR_GIT4  := `git log HEAD~3 --oneline --decorate=no -n 1`
CBR_GIT5  := `git log HEAD~4 --oneline --decorate=no -n 1`

CBR = "\#define BUILDID __attribute__((section(\".buildid\")))\nconst char BUILDID build_id_rom[] = \""'\\'"\nUserLM32"'\\n\\'"\nProject     : $(TARGET)"'\\n\\'"\nVersion     : $(VERSION)"'\\n\\'"\nPlatform    : $(CBR_PF)"'\\n\\'"\nBuild Date  : $(CBR_DATE)"'\\n\\'"\nPrepared by : $(USER) $(CBR_USR) <$(CBR_MAIL)>"'\\n\\'"\nPrepared on : $(CBR_HOST)"'\\n\\'"\nOS Version  : $(CBR_OS) $(CBR_KRNL)"'\\n\\'"\nGCC Version : $(CBR_GCC)"'\\n\\'"\nFW-ID ROM will contain:"'\\n\\n\\'"\n   $(CBR_GIT1)"'\\n\\'"\n   $(CBR_GIT2)"'\\n\\'"\n   $(CBR_GIT3)"'\\n\\'"\n   $(CBR_GIT4)"'\\n\\'"\n   $(CBR_GIT5)"'\\n\\'"\n\";\n"
