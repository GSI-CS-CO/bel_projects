TARGET		= scu_diob2
DEVICE		= EP2AGX125
FLASH		  = EPCS128
RAM_SIZE	= 32768
PHK       = ../../modules/scu_bus/housekeeping_sw
PDIOB     = ../../top/scu_diob2
W1        = ../../ip_cores/wrpc-sw
CFLAGS    = -I$(PHK)/include -I$(PDIOB)
USRCPUCLK = 125000

include ../build.mk

$(TARGET).mif:  housekeeping.mif

housekeeping.elf: $(PHK)/main.c $(W1)/dev/w1.c $(W1)/dev/w1-temp.c $(W1)/dev/w1-hw.c

check_blackbox:
	@if [ ! -d ../../top/scu_diob2/blackbox ]; then \
		echo ""; \
		echo -e "\033[31m ************************************************* \033[0m"; \
		echo -e "\033[31m        Blackbox directory does not exist!         \033[0m"; \
		echo -e "\033[31m  Will be created now, prepare your gsi git login! \033[0m"; \
		echo -e "\033[31m **************************************************\033[0m"; \
		echo ""; \
		../../top/scu_diob2/clone_blackbox; \
	fi

generate-%: check_blackbox
	../../top/scu_diob2/blackbox/hdl/generate_all $*

all_gen-%: generate-%
	@echo ""
	@echo -e "\033[93m *******************************************\033[0m"
	@echo -e "\033[93m Compiling Blackbox with configuration: $* \033[0m"
	@echo -e "\033[93m *******************************************\033[0m"
	@echo ""

	
	$(MAKE) all

clean::
	rm -f $(POW)/*.o $(PHK)/main.o $(W1)/dev/*.o
