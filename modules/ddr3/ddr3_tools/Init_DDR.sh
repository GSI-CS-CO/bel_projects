# !/bin/bash
# Script  init_ddr.sh : Purpose of this script is to write some data into DDR and check back

# Posix didnt support any arrays or for loops, Posix supports only while loops
echo "-----------------------------------------------------------------------------------------"
echo "Purpose of this script is to check the Transparent Mode"
echo "as well as the Burst read mode of the SCU DDR3 I/F"
echo "-----------------------------------------------------------------------------------------"
echo "In Transparent Mode the DDR starts at address offset 0x0 (which is a LW address)"
echo "and ends at address offset 0x7ffffe8 (which is a LW address too)"
echo "High word addresses need to be pre-loaded, before LW access triggers the DRR write"
echo "-----------------------------------------------------------------------------------------"
echo "In Burst Read Mode, two things had to be done before the fifo starts filling itself:"
echo "a) Write the Burst_Start_Address Register (WB DDR IF1, Address Offset 0x7ff fff0) "
echo "   Burst_start_address values are located between 0x0 and 0x1FF FFFD "
echo ""
echo "b) Write the # of 64 bit transfers to the Xfer_Cntr_Reg (WB DDR IF 1, Address Offset 0x7FF FFF4)"
echo "   Shortly after loading ther Xfer_Cntr_Reg, the Fifo filling process starts by itself"
echo "-----------------------------------------------------------------------------------------"
echo "Then readout of the fifo can be started by steps c to e in a loop"
echo "c) Readout of the Fifo Status Register (WB DDR IF2, Address Offset 0x38)"
echo "   Bit 32     = Fifo Empty"
echo "   Bit 31     = Local Init done"
echo "   Bit 9 to 0 = Used (remaining) FIFO Words"

echo "d) Readout of the  FIFO LW Data (WB DDR IF2. Address Offset 0x30)"
echo "e) Readout of the  FIFO HW Data (WB DDR IF2. Address Offset 0x34)"
echo "Each Fifo Low Word access moves the FIFO Pointer"
echo "-----------------------------------------------------------------------------------------"

wb_ddr3_if1_baseadr=$(eb-find dev/wbm0 0x651 0x20150828)
wb_ddr3_if2_baseadr=$(eb-find dev/wbm0 0x651 0x20160525)

echo "SCU DDR3 IF1 baseadr is:" ${wb_ddr3_if1_baseadr}
echo "SCU DDR3 IF2 baseadr is:" ${wb_ddr3_if2_baseadr}

if [ "$wb_ddr3_if1_baseadr" = "" ]; then
    exit
fi

if [ "$wb_ddr3_if2_baseadr" = "" ]; then
    exit
fi


echo "-----------------------Transparent Mode--------------------------"

echo "First: readout of the initial value of some DDR3 cells at start/end of the address range"

adr1_LW=$wb_ddr3_if1_baseadr+0x0
adr1_HW=$wb_ddr3_if1_baseadr+0x4


adr2_LW=$wb_ddr3_if1_baseadr+0x8
adr2_HW=$wb_ddr3_if1_baseadr+0xc

adr3_LW=$wb_ddr3_if1_baseadr+0x10
adr3_HW=$wb_ddr3_if1_baseadr+0x14

adr4_LW=$wb_ddr3_if1_baseadr+0x18
adr4_HW=$wb_ddr3_if1_baseadr+0x1c

adr5_LW=$wb_ddr3_if1_baseadr+0x20
adr5_HW=$wb_ddr3_if1_baseadr+0x24

adr6_LW=$wb_ddr3_if1_baseadr+0x28
adr6_HW=$wb_ddr3_if1_baseadr+0x2c

adr7_LW=$wb_ddr3_if1_baseadr+0x7ffffe8
adr7_HW=$wb_ddr3_if1_baseadr+0x7ffffec



rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr1_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr1_HW))  )/4")
echo    "adr HW  initial Value = "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr2_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr2_HW))  )/4")
echo    "adr HW  initial Value = "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr3_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr3_HW))  )/4")
echo    "adr HW  initial Value = "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr4_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr4_HW))  )/4")
echo    "adr HW  initial Value = "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr5_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr5_HW))  )/4")
echo    "adr HW  initial Value = "$rdval


rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr6_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr6_HW))  )/4")
echo    "adr HW  initial Value = "$rdval


rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr7_LW))  )/4")
echo    "adr LW  initial Value = "$rdval
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr7_HW))  )/4")
echo    "adr HW  initial Value = "$rdval

echo ""
echo ""

echo "Second: write some values starting cafecaf0, cafecaf4 .etc, preload HW first ! "

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr1_HW))  )/4"  0xcafebab4
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr1_LW))  )/4"  0xcafebab0

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr2_HW))  )/4"  0xcafebabc
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr2_LW))  )/4"  0xcafebab8

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr3_HW))  )/4"  0xcafeba14
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr3_LW))  )/4"  0xcafeba10

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr4_HW))  )/4"  0xcafeba1c
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr4_LW))  )/4"  0xcafeba18

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr5_HW))  )/4"  0xcafeba24
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr5_LW))  )/4"  0xcafeba20

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr6_HW))  )/4"  0xcafeba2c
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr6_LW))  )/4"  0xcafeba28

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr7_HW))  )/4"  0xbadeafec
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr7_LW))  )/4"  0xbadeafe8

echo ""
echo ""

echo "Thirdly: readback just written values starting with cafebab0"


echo "1"
rdval1=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr1_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval1
rdval2=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr1_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval2

echo "2"
rdval3=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr2_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval3
rdval4=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr2_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval4

echo "3"
rdval5=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr3_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval5
rdval6=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr3_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval6

echo "4"
rdval7=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr4_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval7
rdval8=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr4_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval8

echo "5"
rdval9=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr5_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval9
rdval10=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr5_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval10

echo "6"
rdval11=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr6_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval11
rdval12=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr6_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval12

echo "7"
rdval13=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr7_LW))  )/4")
echo    "adr LW  readback written Value = "$rdval13
rdval14=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr7_HW))  )/4")
echo    "adr HW  readback written Value = "$rdval14


echo ""
echo ""

echo "Fourthly:-----------------Testing Burst Mode--------------------------"

adr_burst_startadr_reg=$wb_ddr3_if1_baseadr+0x7fffff4
adr_xfer_cnt_adr=$wb_ddr3_if1_baseadr+0x7fffff8

adr_fifo_LW_data=$wb_ddr3_if2_baseadr+0x30
adr_fifo_HW_data=$wb_ddr3_if2_baseadr+0x34
adr_fifo_status=$wb_ddr3_if2_baseadr+0x38

echo "------------------show initial state of Burst readout control registers----------------"
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_burst_startadr_reg))  )/4")
echo    "burst_startadr_reg Value= "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_xfer_cnt_adr))  )/4")
echo    "xfer_cnt Value= "$rdval

echo "readout an empty FIFO will be echoed with wishbone error"
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "FIFO LW= "$rdval

echo ""
echo "set up a 3-word burst transfer (each word is 64 bits and needs 2 fifo readouts)"
echo ""
eb-write dev/wbm0                "0x$( printf "%X\n" $((adr_burst_startadr_reg))  )/4"  0x2
echo    "setting start address to 2"

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_burst_startadr_reg))  )/4")
echo    "burst_startadr_reg Value= "$rdval

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr_xfer_cnt_adr))  )/4"  0x3
echo    "setting xfer count to 3"

echo ""
echo ""

echo "Fiftly:------------fifo readout sequence of a 3-word burst-------------------------"
rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_xfer_cnt_adr))  )/4")
echo    "xfer_cnt Value now zero because immediately processed after set =>"$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

echo "----------------------------------------------------------------------------------"
echo ""
echo ""

echo "set up a 4-Word-Bursttransfer (checking upper end of the DDR address range)"
echo ""

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status  = "$rdval

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr_burst_startadr_reg))  )/4"  0x1FFFFFD
echo    "setting start address to 0x 1FF FFF D"

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_burst_startadr_reg))  )/4")
echo    "burst_startadr_reg Value= "$rdval
echo "----------------------------------------------------------------------------------"
echo " This is fooling the DDR3 Ram by reading the last valid date at 0x1FF FFFD"
echo " and two not by wishbone addressable DDR3 Cells at 0x1FF FFFE and 0x1FF FFFF           "
echo " as well as Adr 0x0 (the overflow value of Adr 0x1FFFFF) with contents 0xcafebab0/0xcafebabe4"
echo "----------------------------------------------------------------------------------"

eb-write dev/wbm0                "0x$( printf "%X\n" $((adr_xfer_cnt_adr))  )/4"  0x4
echo    "setting xfer count to 0x4 64-bit word"

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval


rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_LW_data))  )/4")
echo    "read low word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_HW_data))  )/4")
echo    "read high word from fifo  ="$rdval

rdval=$(eb-read  dev/wbm0             "0x$( printf "%X\n" $((adr_fifo_status))  )/4")
echo    "fifo_status Value = : "$rdval


