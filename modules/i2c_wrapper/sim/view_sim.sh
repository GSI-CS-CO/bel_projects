#!/bin/bash
source settings.sh

# View
echo $(md5sum $VCD_NAME)
ls -la $VCD_NAME
$VCD_VIEWER $VCD_NAME $GTKW_NAME &
