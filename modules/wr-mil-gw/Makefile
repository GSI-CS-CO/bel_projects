TARGET_DIR=/common/export

# for prductive deployment,

all:
	make PATH=../../../toolchain/bin:$(PATH) -C firmware 
	make MASP=YES PRO=NO PREFIX= -C saft-control

pro:
	make PATH=../../../toolchain/bin:$(PATH) -C firmware 
	make MASP=YES PRO=YES PREFIX= -C saft-control

clean:
	make -C firmware clean
	make -C saft-control clean

deploy-dev:
	# init script
	mkdir -p $(TARGET_DIR)/nfsinit/global
	cp setup/nfsinit/global/timing-rte-wrmilgw-ESR-dev $(TARGET_DIR)/nfsinit/global/
	cp setup/nfsinit/global/timing-rte-wrmilgw-SIS-dev $(TARGET_DIR)/nfsinit/global/

	# timing-rte ESR version
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-ESR-dev/firmware
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-ESR-dev/x86_64/usr/bin
	cp -r setup/timing-rte/wrmilgw-ESR-dev $(TARGET_DIR)/timing-rte/
	cp firmware/wr_mil.bin $(TARGET_DIR)/timing-rte/wrmilgw-ESR-dev/firmware/
	cp control/wr-mil-gw-ctl $(TARGET_DIR)/timing-rte/wrmilgw-ESR-dev/x86_64/usr/bin/

	# timing-rte SIS version
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-SIS-dev/firmware
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-SIS-dev/x86_64/usr/bin
	cp -r setup/timing-rte/wrmilgw-SIS-dev $(TARGET_DIR)/timing-rte/
	cp firmware/wr_mil.bin $(TARGET_DIR)/timing-rte/wrmilgw-SIS-dev/firmware/
	cp control/wr-mil-gw-ctl $(TARGET_DIR)/timing-rte/wrmilgw-SIS-dev/x86_64/usr/bin/

deploy-pro:
	# init script
	mkdir -p $(TARGET_DIR)/nfsinit/global
	cp setup/nfsinit/global/timing-rte-wrmilgw-ESR $(TARGET_DIR)/nfsinit/global/
	cp setup/nfsinit/global/timing-rte-wrmilgw-SIS $(TARGET_DIR)/nfsinit/global/

	# timing-rte ESR version
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-ESR/firmware
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-ESR/x86_64/usr/bin
	cp -r setup/timing-rte/wrmilgw-ESR-dev/* $(TARGET_DIR)/timing-rte/wrmilgw-ESR/
	cp firmware/wr_mil.bin $(TARGET_DIR)/timing-rte/wrmilgw-ESR/firmware/
	cp control/wr-mil-gw-ctl $(TARGET_DIR)/timing-rte/wrmilgw-ESR/x86_64/usr/bin/

	# timing-rte SIS version
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-SIS/firmware
	mkdir -p $(TARGET_DIR)/timing-rte/wrmilgw-SIS/x86_64/usr/bin
	cp -r setup/timing-rte/wrmilgw-SIS-dev/* $(TARGET_DIR)/timing-rte/wrmilgw-SIS/
	cp firmware/wr_mil.bin $(TARGET_DIR)/timing-rte/wrmilgw-SIS/firmware/
	cp control/wr-mil-gw-ctl $(TARGET_DIR)/timing-rte/wrmilgw-SIS/x86_64/usr/bin/

