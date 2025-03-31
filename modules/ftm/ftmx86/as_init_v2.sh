#!/bin/sh

# Init DM for message reception. Bind DM memory node INBOX @ "registers" to eca wb sink
# Copy data from timing message param field to Inbox on reception

#               cDCTT  command DM CPU Thr
#IC =0x14c0fc0000000000  # Timing ID condition action sink will trigger on
#TIC=0x14d0d00
#MSK=0xfffffff000000000  # Filter mask for Timing ID Condition

FID=0x1
GIDBASE=0x4d0
EVTNOBASE=0xd00

to_upper() {
  echo "$1" | tr '[:lower:]' '[:upper:]'
}

# Function to convert hex to decimal
hex_to_dec() {
  printf "%d" "$1"
}

# Function to convert decimal to 64-bit hex
dec_to_hex32() {
  printf "0x%08x\n" "$1"
}

dec_to_hex64() {
  printf "0x%016x\n" "$1"
}

# Function to shift left using bc
shift_left() {
  RESULT=$(awk "BEGIN {print $1 * 2^$2}")
  echo "$RESULT"
}

sumHex() {
  DEC_A=$(hex_to_dec "$1")
  DEC_B=$(hex_to_dec "$2")
  RESULT=$(awk "BEGIN {print $DEC_A + $DEC_B}")
  dec_to_hex64 "$RESULT"
}


sum() {
  DEC_A="$1"
  DEC_B="$2"
  RESULT=$(awk "BEGIN {print $DEC_A + $DEC_B}")
  echo "$RESULT"
}


create_TIC() {

  A=$(sum  "0" $(shift_left $(hex_to_dec $FID)       60))
  A=$(sum "$A" $(shift_left $(hex_to_dec $GIDBASE)   48))
  A=$(sum "$A" $(shift_left $(hex_to_dec $2)         48)) #dm
  A=$(sum "$A" $(shift_left $(hex_to_dec $EVTNOBASE) 36))
  A=$(sum "$A" $(shift_left $(hex_to_dec $1)         36)) #command
  A=$(sum "$A" $(shift_left $(hex_to_dec $3)         32)) #cpu
  A=$(sum "$A" $(shift_left $(hex_to_dec $4)         20)) #thr

  echo $(dec_to_hex64 "$A")
}

create_PFMSK() {            ####    ####
  BASEMSK=$(printf "%d" "-1")

  REV=$(awk "BEGIN {print 64 - $1}")
  REM=$(shift_left "1" "$REV")
  SUB=$(awk "BEGIN {print $BASEMSK - ($REM - 1)}")
  
  
  echo $(dec_to_hex64 "$SUB")
  
}

create_graph() {
  printf 'digraph g {\n name="Action Sink Init";\n graph[]\n\n%s \n}' "$1"  
}

create_global_node() {
  LCPU="$1"
  LSEC="$2"
  LNAME="$3"


  printf ' %s [cpu="%s", type="global", section="%s"];\n' "$LNAME" "$LCPU" "$LSEC"
  printf ' ABLOCK [cpu="%s", type="block", tperiod=10000, pattern="EXTERN", patentry="true", patexit="true"];\n' "$LCPU"
  printf ' ABLOCK -> %s [type="reference", fieldhead="0x10", fieldtail="0x0", fieldwidth="32"];\n' "$LNAME"
  
  unset LCPU LSEC LNAME
}

get_adr() { #eb-dev #dotfile #adrfile
  DC="dm-cmd"
  DS="dm-sched"

  LDEV="$1"
  LDOTFILE="$2"
  LADRRAWFILE="$3"
  LNAME="$4"

  $DC $LDEV halt
  sleep 0.2
  $DS $LDEV clear
  sleep 0.2
  $DS $LDEV add /tmp/$LDOTFILE
  sleep 0.2
  $DS $LDEV status > /tmp/$LADRRAWFILE
  
  LADR=$(awk -v name="$LNAME" '$0 ~ name {print $NF}' /tmp/$LADRRAWFILE)
  sleep 0.2
  echo "$LADR"

  unset DC DS LDEV LDOTFILE LADRRAWFILE LNAME

}

clear_all_macros() {
  SWC="saft-wbm-ctl"
  TR="$1"
  echo "$SWC $TR -x; sleep 0.2;"
}

set_64b_macro() { #TR #ADR/64 #MACRO #TIC #MSK
  SWC="saft-wbm-ctl"
  TR="$1"                     #timing receiver name, ie 'tr0' etc
  LADR_AUX=$(hex_to_dec "$2")
  LADR_HI=$(dec_to_hex32 "$LADR_AUX") #64b aligned address/address of the high word
  LADR_LO=$(dec_to_hex32 $(sum "$LADR_AUX" "4")) #add 4 byte to Hi word gives us low word address
  LMACRO_HI="$3"
  LMACRO_LO=$(sum  "1" "$3")  #add 1 to index of hi word macro gives us low word macro index
  LTIC="$4"                   #timing ID
  LMSK="$5"                   #MSK
  echo "$SWC $TR -r $LMACRO_HI $LADR_HI 0 0x4f; sleep 0.2;"
  echo "$SWC $TR -r $LMACRO_LO $LADR_LO 0 0x5f; sleep 0.2;"
  echo "$SWC $TR -c $LTIC $MSK 0     $LMACRO_HI -d;"
  echo "$SWC $TR -c $LTIC $MSK 10000 $LMACRO_LO -d;"
  echo "sleep 0.5;"
  echo "$SWC $TR -l"

}



CMD=0x3
DM=0x1
CPU=0
THR=1
SECSTR="threadStaging"

DOTFILE=tmp.dot
ADRRAWFILE=adrraw.txt
ADRFILE=adr.txt
DEV=dev/wbm0
TR=tr0

THRSTR=$(printf "%02d" "$THR")
#SEC="$SECSTR"_"$THRSTR"
SEC="threadControl"
NAME=$(to_upper "C"_"$CPU"_"$SEC")
NODE=$(create_global_node "$CPU" "$SEC" "$NAME")
echo $(create_graph "$NODE") > /tmp/$DOTFILE
#
TIC=$(create_TIC "$CMD" "$DM" "$CPU" "$THR")
MSK=$(create_PFMSK 64)
ADR=$(get_adr "$DEV" "$DOTFILE" "$ADRRAWFILE" "$NAME")
MACRO=1

#
#
clear_all_macros "$TR"
set_64b_macro "$TR" "$ADR" "$MACRO" "$TIC" "$MSK"


echo "$TIC"
echo "$MSK"
echo "$ADR"
#echo $(shift_left "4" "2")

##eb-write $DEV $ADR/4 0x0
##
#ASCMD="saft-wbm-ctl $TR -x; saft-wbm-ctl $TR -r 3 $ADR 0 0x5f; saft-wbm-ctl $TR -c $TIC $MSK 0 3 -d; sleep 1; saft-wbm-ctl $TR -l"
#
#
#saft-wbm-ctl tr1 -x; sleep 0.2;
#saft-wbm-ctl tr1 -r 0 0x4060404 0 0x5f; sleep 0.2;
#saft-wbm-ctl tr1 -r 1 0x4060400 0 0x4f; sleep 0.2;
#saft-wbm-ctl tr1 -c 0x14d1d03200500000 0xffffffffffffffff 0 0 -d;
#saft-wbm-ctl tr1 -c 0x14d1d03200500000 0xffffffffffffffff 10000 1 -d;
#sleep 1; saft-wbm-ctl tr1 -l


#eval $ASCMD
#
#echo "Setting up action sink: $ASCMD"
#
#printf "$TIC $INITVAL 0" > /tmp/unilac_20.txt; sleep 0.1; saft-dm $TR -p -v /tmp/unilac_20.txt
#dm-sched $DEV clear
#sleep 0.5
#VAL=`eb-read $DEV $ADR/4`
#echo "ADR $ADR now contains INITVAL 0x$VAL"

