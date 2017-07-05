#~/bin/bash

all:
	cd control; make
	cd firmware; make

deploy:
	# init script
	cp    setup/nfsinit/global/* /common/export/nfsinit/global/

	# timing-rte ESR version
	cp -r setup/timing-rte/wrmilgw-ESR-dev /common/export/timing-rte/
	cp firmware/wr_mil.bin /common/export/timing-rte/wrmilgw-ESR-dev/firmware/

	# timing-rte SIS version
	cp -r setup/timing-rte/wrmilgw-SIS-dev /common/export/timing-rte/
	cp firmware/wr_mil.bin /common/export/timing-rte/wrmilgw-SIS-dev/firmware/

