#!/usr/bin/env bash
# Start OpenOCD for the NEORV32 debug module (Altera virtual JTAG / SLD hub).
# GDB server on port 3333, see README.md.
#
# Usage: ./start-openocd.sh [openocd args...]
#
# Before OpenOCD is started, the JTAG server is prepared:
#   1. set the cable's JTAG clock (the SLD hub is not found at the default 24 MHz;
#      jtagd forgets the setting when the cable is replugged or jtagd restarts)
#   2. check that "jtagconfig -n" lists the NEORV32 debug node
#   3. if not: restart a local jtagd (it sometimes hangs and then shows the devices
#      without names and no SLD nodes), set the clock again and check again
#   4. read the device list of the chain: a CoolRunner-II CPLD (IDCODE 06E5D093,
#      e.g. on pexp) before or after the FPGA is declared to OpenOCD automatically
#      (PRE_TAPS/POST_TAPS); other unknown devices are an error
#
# Environment:
#   QUARTUS     Quartus installation with OpenOCD (Quartus >= 23, Altera fork with
#               aji_client), e.g. QUARTUS=/opt/quartus/23/quartus
# Optional:
#   OPENOCD     OpenOCD binary (default: $QUARTUS/bin/openocd)
#   JTAGCONFIG  jtagconfig binary (default: $QUARTUS/bin/jtagconfig)
#   CABLE       jtagd cable name as printed by jtagconfig (default: USB-BlasterII [1-6])
#   JTAG_CLOCK  JTAG clock (default: 6M)
#   VJTAG_ID    SLD node id of the debug module (default: 00406E00)
#   FPGA_ID     IDCODE of the FPGA (default: 02A010DD, Arria V 5AGXMA3D4)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CFG="$SCRIPT_DIR/../../../openocd/neorv32_altera_vjtag.cfg"

OPENOCD="${OPENOCD:-${QUARTUS:+$QUARTUS/bin/openocd}}"
JTAGCONFIG="${JTAGCONFIG:-${QUARTUS:+$QUARTUS/bin/jtagconfig}}"
CABLE="${CABLE:-USB-BlasterII [1-6]}"
JTAG_CLOCK="${JTAG_CLOCK:-6M}"
VJTAG_ID="${VJTAG_ID:-00406E00}"
FPGA_ID="${FPGA_ID:-02A010DD}"

case "$1" in
  -h|--help) sed -n '2,25p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
esac

info() { echo "start-openocd: $*" >&2; }

if [ -z "$OPENOCD" ] || [ -z "$JTAGCONFIG" ]; then
  info "error: QUARTUS is not set (path to the Quartus >= 23 installation, the directory containing bin/openocd)"; exit 1
fi
for b in "$OPENOCD" "$JTAGCONFIG"; do
  if [ ! -x "$b" ]; then
    info "$b not found (OpenOCD needs Quartus >= 23, QUARTUS=$QUARTUS)"; exit 1
  fi
done

# print the "jtagconfig -n" block of $CABLE (from "N) <cable>" to the next cable)
cable_block() {
  "$JTAGCONFIG" -n 2>/dev/null | tr -d '\r' | awk -v c="$CABLE" '
    /^[0-9]+\) / { p = (substr($0, index($0, ") ") + 2) == c) }
    p'
}

set_clock() {
  "$JTAGCONFIG" --setparam "$CABLE" JtagClock "$JTAG_CLOCK" >/dev/null 2>&1 || true
  info "JtagClock of '$CABLE': $("$JTAGCONFIG" --getparam "$CABLE" JtagClock 2>/dev/null | tr -d '\r')"
}

node_found() {
  local blk
  blk="$(cable_block)"
  if [ -z "$blk" ]; then
    info "cable '$CABLE' not found, jtagconfig lists:"
    "$JTAGCONFIG" 2>&1 | tr -d '\r' | grep -E '^[0-9]+\)' >&2 || true
    exit 1
  fi
  grep -qi "Node $VJTAG_ID" <<<"$blk"
}

set_clock
if ! node_found; then
  case "$CABLE" in
    *" on "*)
      info "debug node $VJTAG_ID not found; '$CABLE' is on a remote jtagd - restart jtagd there"
      exit 1 ;;
  esac
  info "debug node $VJTAG_ID not found, restarting jtagd"
  pkill -x jtagd || true
  sleep 2
  "$JTAGCONFIG" >/dev/null 2>&1 || true   # starts jtagd again
  set_clock
  if ! node_found; then
    info "debug node $VJTAG_ID still not found. Chain of '$CABLE':"
    cable_block >&2
    info "is the FPGA configured with a NEORV32 SLD DTM gateware?"
    exit 1
  fi
fi
info "debug node $VJTAG_ID found"

# JTAG chain: device IDCODEs in chain order (lines "  <8 hex digits> [name]")
mapfile -t CHAIN < <(cable_block | awk '/^ +[0-9A-Fa-f]{8}( |$)/ { print toupper($1) }')
info "JTAG chain: ${CHAIN[*]}"
PRE="" POST="" SEEN_FPGA=0 N=0
for id in "${CHAIN[@]}"; do
  if [ "$id" = "${FPGA_ID^^}" ] && [ $SEEN_FPGA = 0 ]; then SEEN_FPGA=1; continue; fi
  case "$id" in
    06E5D093) tap="{cpld$N 8 0x06e5d093}"; N=$((N + 1)) ;;
    *) info "unknown device $id in the JTAG chain (IR length unknown), add it to this script"; exit 1 ;;
  esac
  if [ $SEEN_FPGA = 0 ]; then PRE="$PRE $tap"; else POST="$POST $tap"; fi
done
if [ $SEEN_FPGA = 0 ]; then
  info "FPGA $FPGA_ID not in the JTAG chain"; exit 1
fi

ARGS=(-c "set CABLE {$CABLE}" -c "set FPGA_ID 0x${FPGA_ID,,}")
if [ -n "$PRE" ];  then info "CPLD before the FPGA"; ARGS+=(-c "set PRE_TAPS {${PRE# }}"); fi
if [ -n "$POST" ]; then info "CPLD after the FPGA";  ARGS+=(-c "set POST_TAPS {${POST# }}"); fi

exec "$OPENOCD" "${ARGS[@]}" -f "$CFG" "$@"
