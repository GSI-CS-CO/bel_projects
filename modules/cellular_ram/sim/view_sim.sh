#!/bin/bash
source settings.sh

# View
echo "Viewing $TB_NAME ..."
#echo $(md5sum $VCD_NAME)
echo $(md5sum $GHW_NAME)
#ls -la $VCD_NAME
ls -la $GHW_NAME
#$VCD_VIEWER $VCD_NAME $GTKW_NAME
$VCD_VIEWER $GHW_NAME $GTKW_NAME
