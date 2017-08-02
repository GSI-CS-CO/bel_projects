TARGET_DIR=/common/export

all:
	make PATH=../../../toolchain/bin:$(PATH) -C firmware 
	make -C control

deploy:
	# init script
	cp    setup/nfsinit/global/* $(TARGET_DIR)/nfsinit/global/

	# timing-rte ESR version
	cp -r setup/timing-rte/wrmilgw-ESR-dev $(TARGET_DIR)/timing-rte/
	cp firmware/wr_mil.bin $(TARGET_DIR)/timing-rte/wrmilgw-ESR-dev/firmware/
	cp control/wr-mil-gw-ctl $(TARGET_DIR)/timing-rte/wrmilgw-ESR-dev/x86_64/usr/bin/

	# timing-rte SIS version
	cp -r setup/timing-rte/wrmilgw-SIS-dev $(TARGET_DIR)/timing-rte/
	cp firmware/wr_mil.bin $(TARGET_DIR)/timing-rte/wrmilgw-SIS-dev/firmware/
	cp control/wr-mil-gw-ctl $(TARGET_DIR)/timing-rte/wrmilgw-SIS-dev/x86_64/usr/bin/

