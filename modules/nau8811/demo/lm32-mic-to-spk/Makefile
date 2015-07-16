# Exploder5a Demo
# ====================================================================================================
TARGET				= exploder5_csco_tr
RAM_SIZE			= 65536
USRCPUCLK			= 125000
DEVICE				= 5AGXMA3D4
FLASH					= EPCQ256

BASE_DIR_PATH	= ../../../..
MYPATH				= $(BASE_DIR_PATH)/modules/nau8811/demo/lm32-mic-to-spk
NAU8811_FILES	= $(BASE_DIR_PATH)/modules/nau8811/src/c

CFLAGS				= -I$(MYPATH) -I$(NAU8811_FILES) \
							-W -Wall -Wimplicit -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return \
							-Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wmissing-declarations -Wuninitialized 

include $(BASE_DIR_PATH)/syn/build.mk

$(TARGET).elf: $(MYPATH)/main.c $(INCPATH)/ebm.c $(NAU8811_FILES)/nau8811_audio_driver.c

clean::
	rm -f $(MYPATH)/*.o

test:
			lm32-elf-readelf -hS $(TARGET).elf
			lm32-ctl load $(TARGET).elf

