# Exploder5a Demo
# ====================================================================================================
TARGET				= exploder5_csco_tr
RAM_SIZE			= 65536
USRCPUCLK			= 125000
DEVICE				= 5AGXMA3D4
FLASH					= EPCQ256

BASE_DIR_PATH	= ../../../..
MYPATH				= $(BASE_DIR_PATH)/modules/ssd1325/demo/exploder5_demo
SSD1325_FILES	= $(BASE_DIR_PATH)/modules/ssd1325/src/c

CFLAGS				= -I$(MYPATH) -I$(SSD1325_FILES) \
							-W -Wall -Wimplicit -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return \
							-Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wmissing-declarations -Wuninitialized 

include $(BASE_DIR_PATH)/syn/build.mk

$(TARGET).elf: $(MYPATH)/main.c $(INCPATH)/ebm.c $(SSD1325_FILES)/ssd1325_serial_driver.c

clean::
	rm -f $(MYPATH)/*.o

test:
			lm32-elf-readelf -hS $(TARGET).elf
			../../../../tools/lm32-ctl load $(TARGET).elf

