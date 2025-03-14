# PREFIX  controls where programs and libraries get installed
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all

PREFIX		?= /usr/local
STAGING		?=
EB		?= ../../ip_cores/etherbone-core/api
WRPC		?= ../../ip_cores/wrpc-sw
TARGETS		:= simple-display
USE_RPATH	?= yes
SOURCES		= main-x86.c disp.c disp-oled.c disp-lcd.c ssd1325_serial_driver.c


EXTRA_FLAGS	?=
CFLAGS		?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) #-static-libstdc++
ifeq ($(USE_RPATH),yes)
LIBS		?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm
else
LIBS		?= -letherbone -lm
endif

# special for simple-display

DEFINES		= -DSSD1325_TARGET_EMBEDDED=0


all: simple-display

simple-display: $(SOURCES)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS) -I../../modules/oled_display

install:
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

clean:
	rm -f *.o $(TARGETS)
