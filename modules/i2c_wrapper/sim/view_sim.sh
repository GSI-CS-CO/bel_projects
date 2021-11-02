#!/bin/bash
source settings.sh

# View
echo "Viewing $TB_NAME ..."
echo $(md5sum $VCD_NAME)
ls -la $VCD_NAME
$VCD_VIEWER $VCD_NAME $GTKW_NAME &
