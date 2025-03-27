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
dec_to_hex() {
  printf "0x%016x\n" "$1"
}

# Function to shift left using bc
shift_left() {
  RESULT=$(echo "$1 * (2^$2)" | bc)
  echo "$RESULT"
}

sumHex() {
  DEC_A=$(hex_to_dec "$1")
  DEC_B=$(hex_to_dec "$2")
  RESULT=$(echo "$DEC_A + $DEC_B" | bc)
  dec_to_hex "$RESULT"
}


sum() {
  DEC_A="$1"
  DEC_B="$2"
  RESULT=$(echo "$DEC_A + $DEC_B" | bc)
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

  echo $(dec_to_hex "$A")
}

create_PFMSK() {            ####    ####
  BASEMSK=$(printf "%d" "-1")

  REV=$(echo "64 - $1" | bc)
  REM=$(shift_left "1" "$REV")
  SUB=$(echo "$BASEMSK - ($REM - 1)" | bc)
  
  
  echo $(dec_to_hex "$SUB")
  
}

create_graph() {
  printf 'digraph g {\\n name="Action Sink Init";\\n graph[]\\n\\n%s \\n}' "$1"  
}

create_global_node() {
  LCPU="$1"
  LSEC="$2"
  LNAME="$3"

  printf ' %s [cpu="%s", type="global", section="%s", pattern="EXTERN" ];\\n' "$LNAME" "$LCPU" "$LSEC"

  unset LCPU LSEC LNAME
}
  tr1 -x; sleep 0.2;

set_64b_macro() {
  SWC="saft-wbm-ctl"
  TR="$1"
  LA0="$2"
  LA1="$2"
  LMACRO="$3"
  LTIC
saft-wbm-ctl tr1 -r 0 0x4060404 0 0x5f; sleep 0.2;
saft-wbm-ctl tr1 -r 1 0x4060400 0 0x4f; sleep 0.2;
saft-wbm-ctl tr1 -c 0x14d1d03200500000 0xffffffffffffffff 0 0 -d;
saft-wbm-ctl tr1 -c 0x14d1d03200500000 0xffffffffffffffff 10000 1 -d;
sleep 1; saft-wbm-ctl tr1 -l


}



CMD=0x3
DM=0x1
CPU=2
THR=5
SECSTR="threadStaging"
DOTFILE=tmp.dot
ADRRAWFILE=adrraw.txt
ADRFILE=adr.txt
DEV=dev/ttyUSB0
TR=tr0
SWC=saft-wbm-ctl

THRSTR=$(printf "%02d" "$THR")
SEC="$SECSTR"_"$THRSTR"
NAME=$(to_upper "C"_"$CPU"_"$SEC")
NODE=$(create_global_node "$CPU" "$SEC" "$NAME")

echo $(create_graph "$NODE") > /tmp/$DOTFILE

TIC=$(create_TIC "$CMD" "$DM" "$CPU" "$THR")

MSK=$(create_PFMSK 64)




dm-cmd $DEV halt
sleep 0.2
dm-sched $DEV clear; sleep 0.2
dm-sched $DEV add /tmp/$DOTFILE; sleep 0.2
dm-sched $DEV status > /tmp/$ADRRAWFILE

ADR=$(awk -v name="$NAME" '$0 ~ name {print $NF}' /tmp/$ADRRAWFILE)
sleep 0.2
printf '%s' "$ADR" > /tmp/$ADRFILE
echo "ADR is $ADR. Clearing..."
#eb-write $DEV $ADR/4 0x0
#
ASCMD="saft-wbm-ctl $TR -x; saft-wbm-ctl $TR -r 3 $ADR 0 0x5f; saft-wbm-ctl $TR -c $TIC $MSK 0 3 -d; sleep 1; saft-wbm-ctl $TR -l"


saft-wbm-ctl tr1 -x; sleep 0.2;
saft-wbm-ctl tr1 -r 0 0x4060404 0 0x5f; sleep 0.2;
saft-wbm-ctl tr1 -r 1 0x4060400 0 0x4f; sleep 0.2;
saft-wbm-ctl tr1 -c 0x14d1d03200500000 0xffffffffffffffff 0 0 -d;
saft-wbm-ctl tr1 -c 0x14d1d03200500000 0xffffffffffffffff 10000 1 -d;
sleep 1; saft-wbm-ctl tr1 -l


#eval $ASCMD
#
echo "Setting up action sink: $ASCMD"
#
#printf "$TIC $INITVAL 0" > /tmp/unilac_20.txt; sleep 0.1; saft-dm $TR -p -v /tmp/unilac_20.txt
#dm-sched $DEV clear
#sleep 0.5
#VAL=`eb-read $DEV $ADR/4`
#echo "ADR $ADR now contains INITVAL 0x$VAL"

