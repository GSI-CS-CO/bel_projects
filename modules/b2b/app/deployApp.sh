#!/bin/bash
# Example usage: (asl34x)
# './deployApp.sh $HOME/consoleApps/b2b b2b-int-sys-mon /common/usr/timing/htdocs/apps'
# './deployApp.sh $HOME/consoleApps/b2b b2b-pro-sys-mon /common/usr/timing/htdocs/apps'
# './deployApp.sh $HOME/consoleApps/b2b b2b-int-transfer-mon /common/usr/timing/htdocs/apps'
# './deployApp.sh $HOME/consoleApps/b2b b2b-pro-transfer-mon /common/usr/timing/htdocs/apps'

APPBUILD=$1/$2/current
NAME=$2
APPDEPLOY=$3/$2/current

echo 'build path is' $APPBUILD
echo 'deploy path is' $APPDEPLOY

cp $APPBUILD/$NAME.zip $APPDEPLOY
