# build and deploy the dm-monitor app for INT

# Example usage (asl7xy)
# 'NAME=dm-monitor-int BIN=dm-monitor make build'
# Example usage (asl34x)
# 'NAME=dm-monitor-int BIN=dm-monitor make deploy'

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  EBPATH =/common/export/timing-rte/tg-fallout-v6.2.x-rocky8/x86_64
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  EBPATH =/common/export/timing-rte/tg-fallout-v6.2.0/x86_64
else
  EBPATH =/usr/local
endif
endif

BOOSTPATH ?= .
NAME ?=dm-monitor-int
BIN ?=dm-monitor
APPBUILD ?=./$(NAME)
APPDEPLOY ?=/common/usr/timing/htdocs/apps/$(BIN)/$(NAME)

build: clean
	echo 'build path is' $(APPBUILD)
	echo 'building ...'
	mkdir -p $(APPBUILD)
	cp -r ../bin/ $(APPBUILD)/bin/
	cp start-app-$(NAME).sh $(APPBUILD)/bin/
	cp -r ../lib/ $(APPBUILD)/lib/
	cp $(EBPATH)/lib/libetherbone.so.5.1.2 $(APPBUILD)/lib/
	ln -sf libetherbone.so.5.1.2 $(APPBUILD)/lib/libetherbone.so.5.1
	ln -sf libetherbone.so.5.1 $(APPBUILD)/lib/libetherbone.so.5
	cp $(BOOST)/lib/libboost_graph.so.1.69.0 $(APPBUILD)/lib/
	cp $(BOOST)/lib/libboost_serialization.so.1.69.0 $(APPBUILD)/lib/
	cp $(BOOST)/lib/libboost_regex.so.1.69.0 $(APPBUILD)/lib/
	zip -rm $(NAME).zip $(APPBUILD)/bin/ $(APPBUILD)/lib/

deploy:
	@echo 'name is ' $(NAME)
	@echo 'build path is' $(APPBUILD)
	@echo 'deploy path is' $(APPDEPLOY)
	cp $(NAME).zip $(APPDEPLOY)

clean:
	@echo 'cleaning ...'
	rm -rf $(NAME).zip $(APPBUILD)/bin/ $(APPBUILD)/lib/

.PHONY: clean build deploy
