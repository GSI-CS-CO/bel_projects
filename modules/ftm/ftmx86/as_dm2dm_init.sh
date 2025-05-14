#!/bin/sh

# Whitelisted keys (space-separated string)
allowed_keys="dm cmd cpu thr width toffs idx dev tr clear field"

# Init DM for message reception. Bind DM memory node to eca wb sink
# Copy data from timing message param field to Inbox on reception

#               cDCTT  command DM CPU Thr
#IC =0x14c0fc0000000000  # Timing ID condition action sink will trigger on
#MSK=0xfffffff000000000  # Filter mask for Timing ID Condition

#Constants
###
SWC="saft-wbm-ctl"
FID=0x1
GIDBASE=0x4d0
EVTNOBASE=0xd00
DOTFILE=tmp.dot
ADRRAWFILE=adrraw.txt
ADRFILE=adr.txt

TOFFS_MACRO2=8

CMD_INBOX=registers
CMD_THR_START=threadControl
CMD_THR_TIME=threadStaging

# 0x0 = data field from record
# 0x1 = channel valid/late/conflict
# 0x2 = channel evt ID high bits
# 0x3 = channel evt ID low bits 
# 0x4 = channel parameter high bits
# 0x5 = channel parameter low bits
# 0x6 = channel tag
# 0x7 = channel tef
# 0x8 = channel time high bits
# 0x9 = channel time low bits

FIELD_ID_HI=0x20
FIELD_PAR_HI=0x40
FIELD_TIME_HI=0x80
###



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
  printf ' ABLOCK -> %s [type="reference", fieldhead="0x0", fieldtail="0x0", fieldwidth="32"];\n' "$LNAME"
  
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

get_cmd_name() { #CMD #CPU #THR
  LCMD=$1
  LCPU=$2
  LTHR=$3

  case "$LCMD" in
    2)
      NAME=$(to_upper "C"_"$LCPU"_"$CMD_INBOX")
      ;;
    1)
      THRSTR=$(printf "%02d" "$LTHR")
      SEC="$CMD_THR_TIME"_"$THRSTR"
      NAME=$(to_upper "C"_"$LCPU"_"$SEC")
      ;;
    0)
      NAME=$(to_upper "C"_"$LCPU"_"$CMD_THR_START")
      ;;
    *)
      echo "Unrecognized CMD"
      exit 1
      ;;
  esac

  
  echo "$NAME"
}

get_cmd_sec() { #CMD #CPU #THR
  LCMD=$1
  LCPU=$2
  LTHR=$3

  case "$LCMD" in
    2)
      SEC="$CMD_INBOX"
      ;;
    1)
      THRSTR=$(printf "%02d" "$LTHR")  
      SEC="$CMD_THR_TIME"_"$THRSTR"
      ;;
    0)
      SEC="$CMD_THR_START"
      ;;
    *)
      echo "Unrecognized CMD"
      exit 1
      ;;
  esac

  
  echo "$SEC"
}


clear_all_macros() {
  TR="$1"
  set -x 
  $SWC $TR -x
  set +x
}

set_64b_macro() { #TR #ADR #MACRO #TIC #MSK #FIELD #TOFFS
  
  TR="$1"                     #timing receiver name, ie 'tr0' etc
  LADR_AUX=$(hex_to_dec "$2")
  LADR_HI=$(dec_to_hex32 "$LADR_AUX") #64b aligned address/address of the high word
  LADR_LO=$(dec_to_hex32 $(sum "$LADR_AUX" "4")) #add 4 byte to Hi word gives us low word address
  #reference message payload fields

  LMACRO_HI="$3"
  LMACRO_LO=$(sum "$LMACRO_HI" "1")  #add 1 to index of hi word macro gives us low word macro index

  LTIC="$4"                   #timing ID
  LMSK="$5"                   #MSK

  LFIELD_AUX=$(hex_to_dec "$6")
  LFIELD_HI=$(dec_to_hex32 "$LFIELD_AUX") #high word aligned address/address of the high word
  LFIELD_LO=$(dec_to_hex32 $(sum "$LFIELD_AUX" "16")) #add 16 to give us low word


  NUM_HI="$7"
  LTOFFS_HI=$([ "$NUM_HI" -lt 0 ] && echo $(( -NUM_HI )) || echo $NUM_HI)
  SIGN_HI=$([ "$NUM_HI" -lt 0 ] && echo "-g" || echo "")

  NUM_LO=$(sum "$NUM_HI" "$TOFFS_MACRO2")
  LTOFFS_LO=$([ "$NUM_LO" -lt 0 ] && echo $(( -NUM_LO )) || echo $NUM_LO)
  SIGN_LO=$([ "$NUM_LO" -lt 0 ] && echo "-g" || echo "")

  set -x
  $SWC $TR -r $LMACRO_HI $LADR_HI 0 $LFIELD_HI
  $SWC $TR $SIGN_HI -c $LTIC $MSK $LTOFFS_HI  $LMACRO_HI -d
  $SWC $TR -r $LMACRO_LO $LADR_LO 0 $LFIELD_LO
  $SWC $TR $SIGN_LO -c $LTIC $MSK $LTOFFS_LO  $LMACRO_LO -d
  $SWC $TR -l
  set +x
}

set_32b_macro() { #TR #ADR #MACRO #TIC #MSK #FIELD #TOFFS

  LTR="$1"                     #timing receiver name, ie 'tr0' etc
  LADR_AUX=$(hex_to_dec "$2")
  LADR=$(dec_to_hex32 "$LADR_AUX") #32b aligned address
  LMACRO="$3"
  LTIC="$4"                   #timing ID
  LMSK="$5"                   #MSK
  LFIELD="$6"                   #MSK
  NUM="$7"
  LTOFFS=$([ "$NUM" -lt 0 ] && echo $(( -NUM )) || echo $NUM)
  SIGN=$([ "$NUM" -lt 0 ] && echo "-g" || echo "")
  
  set -x  
  $SWC $LTR -r $LMACRO $LADR 0 $LFIELD
  $SWC $LTR $SIGN -c $LTIC $MSK $LTOFFS $LMACRO -d
  $SWC $LTR -l
  set +x
}




# Function to check if a key is allowed
is_allowed_key() {
    for ak in $allowed_keys; do
        [ "$ak" = "$1" ] && return 0
    done
    return 1
}

check_unassigned_keys() {
    for ak in $allowed_keys; do
        auxKey=$(to_upper "$ak")
        eval val=\$$auxKey
        if [ -z "$val" ]; then
            echo "Error: Required key '$ak' is missing or empty." >&2
            exit 1
        fi
    done
    return 0
}




#defaults
MSK='0xffffffffffffffff'
WIDTH='32'
DEV='dev/wbm0'
TR='tr0'
CLEAR=0


# Parse args
for arg in "$@"; do
    case "$arg" in
        *=*)
            key="${arg%%=*}"
            value="${arg#*=}"
            if is_allowed_key "$key"; then
                auxKey=$(to_upper "$key")
                eval "$auxKey=\$value"
            else
                echo "Error: Unknown parameter '$key'" >&2
                echo "\n       Supported parameters: $allowed_keys"
                exit 1
            fi
            ;;
        *)
            echo "Error: Invalid argument format '$arg'" >&2
            exit 1
            ;;
    esac
done

check_unassigned_keys

NAME=$(get_cmd_name "$CMD" "$CPU" "$THR")
SEC=$(get_cmd_sec "$CMD" "$CPU" "$THR")
NODE=$(create_global_node "$CPU" "$SEC" "$NAME")
echo $(create_graph "$NODE") > /tmp/$DOTFILE
ID=$(create_TIC "$CMD" "$DM" "$CPU" "$THR")
ADR=$(get_adr "$DEV" "$DOTFILE" "$ADRRAWFILE" "$NAME")


if [ $CLEAR = '1' ]; then
    echo "Clearing all WBM action channel macros"
    clear_all_macros "$TR"
else
    echo "Preserving previous WBM action channel macros"
fi

if [ $WIDTH = '32' ]; then
    echo "Setting up 32b"
    set_32b_macro "$TR" "$ADR" "$IDX" "$ID" "$MSK" "$FIELD" "$TOFFS"
elif [ $WIDTH = '64' ]; then
    #statements
    echo "Setting up 64b"
    set_64b_macro "$TR" "$ADR" "$IDX" "$ID" "$MSK" "$FIELD" "$TOFFS"
else
    echo "Unknown macro width"
    exit 1
fi

# Example usage
#echo "ID = $ID"
#echo "MSK = $MSK"
#echo "WIDTH = $WIDTH"
#echo "TOFFS = $TOFFS"
#echo "IDX = $IDX"
#echo "DEV = $DEV"
#echo "TR = $TR"
#echo "CLEAR = $CLEAR"
#echo "FIELD = $FIELD"


