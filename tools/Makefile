PREFIX  ?= /usr/local
STAGING ?=
EB      ?= ../ip_cores/etherbone-core/api
ECA	?= ../ip_cores/wr-cores/modules/wr_eca
TLU	?= ../ip_cores/wr-cores/modules/wr_tlu
WRPC    ?= ../ip_cores/wrpc-sw
WBDIR   ?= .
#TARGETS := eb-flash eb-info eb-console eb-config-nv eb-time eb-sflash eb-iflash eb-reset eb-daq-dump eb-fwload eb-i2c-master eb-asmi eb-fg-statistic
TARGETS := eb-flash eb-info eb-console eb-config-nv eb-time eb-sflash eb-iflash eb-daq-dump eb-fwload eb-i2c-master eb-asmi eb-fg-statistic
EXTRA_I := eb-flash-secure
USE_RPATH ?= yes

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -std=gnu99 -Wall -O2 -I $(EB)
ifeq ($(USE_RPATH),yes)
LIBS        ?= -L $(EB)/.libs -Wl,-rpath,$(PREFIX)/lib -letherbone -lm
else
LIBS        ?= -letherbone -lm
endif

#all:	$(TARGETS) monitoring
all:	$(TARGETS)

clean:  monitoring-clean
	rm -f $(TARGETS)

#install: monitoring-install
install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

%:	%.cpp
	$(CXX) $(CFLAGS) -o $@ $< $(LIBS)

monitoring: monitoringg

monitoring-install:
	$(MAKE) -C monitoring install

monitoring-clean:
	$(MAKE) -C monitoring clean

monitoringg:
	$(MAKE) -C monitoring EXTRA_FLAGS="$(EXTRA_FLAGS)" all

ecatools: button-game io-test

ecatools-clean:
	rm -f button-game io-test

ecatools-install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp button-game io-test $(STAGING)$(PREFIX)/bin

button-game: button-game.cpp
	g++ $(CFLAGS) -I $(ECA) -I $(TLU) -o button-game button-game.cpp -L $(ECA) -L $(TLU) $(LIBS) -leca -ltlu

io-test: io-test.cpp
	g++ $(CFLAGS) -I $(ECA) -I $(TLU) -o io-test io-test.cpp -L $(ECA) -L $(TLU) $(LIBS) -leca -ltlu

eb-reset: eb-reset.c
	$(CC) $(CFLAGS) -I$(WBDIR) -I$(WRPC)/include -I$(WRPC)/pp_printf $(WBDIR)/wb_api.c  $(WRPC)/dev/w1.c $(WRPC)/dev/w1-temp.c $(WRPC)/dev/w1-eeprom.c $(WRPC)/tools/eb-w1.c -o eb-reset eb-reset.c $(LIBS)

eb-asmi: eb-asmi.c
	$(CC) $(CFLAGS) -I$(WBDIR) -o eb-asmi eb-asmi.c crc8.c crc32.c $(LIBS)

###################################################################################################
# Yocto area -> See https://git.acc.gsi.de/embedded/meta-timing
###################################################################################################

BINDIR ?= /usr/bin

WRPC_SW_BASE = "../../recipe-sysroot/usr/include"
WRPC_SW_INC = "${WRPC_SW_BASE}"
WRPC_SW_DEV = "${WRPC_SW_BASE}/wrpc-sw"

EB_TOOLS_TO_BUILD := eb-flash eb-info eb-console eb-config-nv eb-time eb-sflash eb-iflash eb-reset eb-daq-dump eb-fwload eb-i2c-master eb-asmi eb-fg-statistic monitoring/eb-mon monitoring/eb-massmon
EB_TOOLS_SOURCES  := wb_api.c ${WRPC_SW_DEV}/w1-hw.c ${WRPC_SW_DEV}/w1-temp.c ${WRPC_SW_DEV}/w1.c crc32.c
DISPLAY_SOURCES   := display/main-x86.c display/disp.c display/disp-oled.c display/disp-lcd.c display/ssd1325_serial_driver.c

eb-tools: eb-tools-compile-tools eb-tools-compile-simple-display

eb-tools-compile-tools:
	for ITEM in ${EB_TOOLS_TO_BUILD}; do \
		$(CC) $(CFLAGS) $(LDFLAGS) $$ITEM.c $(EB_TOOLS_SOURCES) -o $$ITEM -DMETA_TIMING\
		-letherbone \
		-I ${WRPC_SW_INC} -I ${WRPC_SW_DEV} -I ${WRPC_SW_DEV}/wrpc-sw/hw -I ${WRPC_SW_INC}/wr-cores -I . -I ${WRPC_SW_BASE}/pp_printf; \
	done

eb-tools-compile-simple-display:
	$(CC) $(CFLAGS) $(LDFLAGS) -DSSD1325_TARGET_EMBEDDED=0 -DPLATFORM=linux $(DISPLAY_SOURCES) -o simple-display \
	-letherbone \
	-I ./display -I ./../modules/oled_display \
	-fcommon

eb-tools-install: eb-tools-install-tools eb-tools-install-simple-display

eb-tools-install-tools:
	install -d $(DESTDIR)$(BINDIR)
	for ITEM in $(EB_TOOLS_TO_BUILD); do \
		install -m 0755 $$ITEM $(DESTDIR)$(BINDIR); \
	done

eb-tools-install-simple-display:
	install -m 0755 simple-display $(DESTDIR)$(BINDIR)
