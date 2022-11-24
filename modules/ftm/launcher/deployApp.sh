#!/bin/bash

# Example usage: (asl34x)
# './deployApp.sh $HOME/consoleApps/b2b b2b-int-sys-mon /common/usr/timing/htdocs/apps'

APPBUILD=$1/$2/current
NAME=$2
APPDEPLOY=$3/$2/current

echo 'name is ' $NAME
echo 'build path is' $APPBUILD
echo 'deploy path is' $APPDEPLOY

cp $APPBUILD/$NAME.zip $APPDEPLOY
