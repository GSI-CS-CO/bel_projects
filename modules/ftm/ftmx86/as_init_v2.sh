#!/bin/sh

# Init DM for message reception. Bind DM memory node INBOX @ "registers" to eca wb sink
# Copy data from timing message param field to Inbox on reception

#               cDCTT  command DM CPU Thr
IC=0x14c0fc0000000000  # Timing ID condition action sink will trigger on
TIC=0x14d0d00
MSK=0xfffffff000000000  # Filter mask for Timing ID Condition

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
  A=$(sum "$A" $(shift_left $(hex_to_dec $EVTNOBASE) 36))
  A=$(sum "$A" $(shift_left $(hex_to_dec $1)         36)) #command
  A=$(sum "$A" $(shift_left $(hex_to_dec $2)         12)) #dm
  A=$(sum "$A" $(shift_left $(hex_to_dec $3)         8)) #cpu
  A=$(sum "$A" $(shift_left $(hex_to_dec $4)         0)) #thr

  echo $(dec_to_hex "$A")
}

create_PFMSK() {            ####    ####
  BASEMSK=$(printf "%d" "-1")

  REV=$(echo "64 - $1" | bc)
  REM=$(shift_left "1" "$REV")
  SUB=$(echo "$BASEMSK - ($REM - 1)" | bc)
  
  
  echo $(dec_to_hex "$SUB")
  
}

create_global_node() {
  LCPU="$1"
  LSEC="$2"
  LSECTION="$LSEC"
  LNAME=$(to_upper "C"_"$LCPU"_"$LSECTION")

  printf '%s [cpu="%s", type="global", section="%s", pattern="EXTERN" ];\n' "$LNAME" "$LCPU" "$LSECTION"

  unset LCPU LTHR LSEC LSECTION LNAME
}

CMD=0x3
DM=0x1
CPU=2
THR=5
SECSTR="threadStaging"
THRSTR=$(printf "%02d" "$THR")
SEC="$SECSTR"_"$THRSTR"

echo $(create_global_node $CPU $SEC)

echo $(create_TIC $CMD $DM $CPU $THR)

echo $(create_PFMSK 64)

#printf 'digraph g {\\n name=\"Action Sink Init\";\\ngraph[]\\n\n%s\n%s\n}\n' "$NODE_DEFINITIONS" "$EDGE_DEFINITIONS"

