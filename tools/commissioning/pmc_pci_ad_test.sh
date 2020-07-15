#!/bin/bash
# Script pmc_pci_test.sh

# ====================================================================================================
# Simple test to cross check PCI Address/Data lines on the PMC FTRN by writing test data 
# to LM32-RAM-User memory via one interface (PCI/USB) and then reading it back via 
# second interface (USB/PCI).


# input parameters
TTYUSB=${1:-ttyUSB0}      # FTRN USB device handle
PCI_DEV=${2:-wbm0}        # FTRN PCI device handle

# get LM32-RAM-User wishbone address
RAM_ADDR=`eb-ls dev/wbm0 | grep RAM-User | awk '{print $3}'`


write_read_check(){
    for (( i=0; i<32; i++ ))
    do
        WRITE_DATA=`printf "%08X" $(( 2**$i ))`
        printf "%2d : $1 write: 0x$WRITE_DATA" $i
        eb-write -p dev/$1 0x$RAM_ADDR/4 0x$WRITE_DATA
        
        #sleep 1
        
        READ_DATA=`eb-read -p  dev/$TTYUSB 0x$RAM_ADDR/4`
        printf "\t$2 read: 0x$READ_DATA"     
        
        if [ "$WRITE_DATA" == "$READ_DATA" ]; then
            printf " - OK\n"
        else
            printf "\n\nRead data not equal to written data! Test failed!\n\n"
            exit
        fi
    done
}

printf "\n--------------------------------------------------------\n"
printf "Checking PCI lines of the PCI device $2 via $1\n"
printf "by writing and reading at address 0x$RAM_ADDR of the LM32-RAM-User\n\n"

printf "Writing pattern to FTRN via PCI and checking via USB\n\n"
write_read_check $PCI_DEV $TTYUSB


printf "\nWriting pattern to FTRN via USB and reading via PCI\n"
write_read_check $TTYUSB $PCI_DEV

printf "\nTest finished successfully!\n\n"

exit

