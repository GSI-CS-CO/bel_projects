# Build the lm32 firmware using the gnu99 standard, because asm() is 
# a GNU extension and not part of the c99 standard.
CC=lm32-elf-gcc
AS=lm32-elf-as
# enable specific lm32 features
LM32_FEATURE_FLAGS = 		\
	-mbarrel-shift-enabled	\
	-mmultiply-enabled		\
	-msign-extend-enabled	
# either undefine U__STDC_HOSTED__ preprocessor macro (which works but leads to warnings)
#NONSTDC_FLAGS = -U__STDC_HOSTED__
# or supply our own stdint.h (just copied from the compiler's include directory)
#NONSTDC_FLAGS = -I./stdc_include
CFLAGS = 					\
	$(NONSTDC_FLAGS)		\
	$(LM32_FEATURE_FLAGS)	\
	-Wall					\
	-Os                     \
	-std=gnu99				

firmware.bin: main.c
	$(CC) $(LM32_FEATURE_FLAGS) $(NONSTDC_FLAGS) $(CFLAGS) -I. -T ram.ld -Wl,--oformat=binary main.c crt0.S -o firmware.bin


