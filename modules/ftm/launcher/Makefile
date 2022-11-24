# build and deploy the dm-monitor app for INT

# Example usage (asl7xy)
# 'NAME=dm-monitor-int BIN=dm-monitor make build'
# Example usage (asl34x)
# 'NAME=dm-monitor-int BIN=dm-monitor make deploy'

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  EBPATH  =/common/export/timing-rte/tg-fallout-v6.2.x-rocky8/x86_64
  BOOST =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  BOOST =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
  BOOST = /lib/x86_64-linux-gnu
endif
endif

APPBUILD ?=./$(NAME)
APPDEPLOY ?=$(BIN)/$(NAME)

build: clean
	echo 'build path is' $(APPBUILD)
	echo 'building ...'
	cp ../bin $(APPBUILD)/bin
	cp start-app-$(NAME).sh $(APPBUILD)/bin
	cp $(EBPATH)/.libs/libetherbone.so.5.1.2 $(APPBUILD)/lib
	cp $(BOOST)/lib/libboost_graph.so $(APPBUILD)/lib
	cp $(BOOST)/lib/libboost_serialization.so $(APPBUILD)/lib
	cp $(BOOST)/lib/libboost_regex.so $(APPBUILD)/lib
	zip -r $(NAME).zip bin lib
# ln -sf $APPBUILD/lib/libetherbone.so.5.1.2 $APPBUILD/lib/libetherbone.so.5.1
# ln -sf $APPBUILD/lib/libetherbone.so.5.1 $APPBUILD/lib/libetherbone.so.5
# cd $APPBUILD

# Example usage: (asl34x)
# './deployApp.sh $HOME/consoleApps/b2b b2b-int-sys-mon /common/usr/timing/htdocs/apps'
deploy:
	echo 'name is ' $(NAME)
	echo 'build path is' $(APPBUILD)
	echo 'deploy path is' $(APPDEPLOY)
	cp $(APPBUILD)/$(NAME).zip $(APPDEPLOY)

clean:
	echo 'cleaning ...'
	rm -r $(APPBUILD)/$(NAME).zip $(APPBUILD)/bin/ $(APPBUILD)/lib/

.PHONY: clean build deploy
