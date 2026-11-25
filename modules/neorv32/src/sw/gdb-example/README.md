# gdb-example

Same program as `idle-init`, but built with `-O0 -g3` so it can be debugged at source level with GDB
over the FPGA's JTAG port (USB-Blaster, Altera virtual JTAG). No extra JTAG or UART cable is needed.

## Prerequisites

- Gateware with NEORV32 and the SLD debug transport module (`modules/neorv32/src/hdl/neorv32_debug_dtm_altera_sld.vhd`),
  e.g. `make pexp-neorv32`.
- OpenOCD from Quartus 23 (Altera fork with `aji_client`); `QUARTUS` must point to that installation (the directory containing `bin/openocd`).
- A Quartus JTAG server (`jtagd`) that sees the cable, local or remote (`jtagconfig --addserver <host[:port]> <password>`).
- `gdb-multiarch` (or `riscv64-unknown-elf-gdb`) (`sudo apt install gdb-multiarch`).

Check that the debug node is visible:

```shell
jtagconfig -n
#   02A010DD   5AG(TMC3D3|XBA3D4|XBA3D6)/..
#     Node 00406E00  (110:8) #0          <- NEORV32 debug module
```

If the `Node` line is missing, lower the JTAG clock (24 MHz did not work through a ribbon/adapter):

```shell
jtagconfig --setparam "USB-BlasterII [1-6]" JtagClock 6M
```

(`start-openocd.sh` does this check and the clock setting automatically.)

### Remote JTAG server through an SSH tunnel

If the USB-Blaster is plugged into another machine (`<jtaghost>`), OpenOCD can use its `jtagd` (TCP port 1309)
through an SSH port forward. Port 1309 does not have to be reachable from the network; only the tunnel endpoint
on the JTAG host's loopback interface is used.

On `<jtaghost>` (once): enable remote access of `jtagd`, which sets the password clients must send:

```shell
jtagconfig --enableremote <password>
```

If the SSH server restricts forwarding, the account needs at least (`/etc/ssh/sshd_config`):

```
AllowTcpForwarding local
PermitOpen 127.0.0.1:1309
```

On the debug machine: open the tunnel (local port 13090 -> `jtagd` on `<jtaghost>`) and register it as a JTAG server:

```shell
ssh -f -N -o ExitOnForwardFailure=yes -L 13090:127.0.0.1:1309 <user>@<jtaghost>
jtagconfig --addserver 127.0.0.1:13090 <password>   # stored in ~/.jtag.conf, needed once
jtagconfig -n                                       # cable shows up as "USB-BlasterII on 127.0.0.1:13090 [1-6]"
```

Use that cable name for the script:

```shell
CABLE='USB-BlasterII on 127.0.0.1:13090 [1-6]' ./start-openocd.sh
```

Notes:
- `jtagconfig --setparam` (JTAG clock) works through the tunnel, but `start-openocd.sh` cannot restart a remote
  `jtagd`. If the devices are listed without names and the debug node is missing, restart `jtagd` on `<jtaghost>`
  (stop it, then run `jtagconfig` there, which starts it again).
- Close the tunnel with `kill` on the `ssh` process (or use `-M -S <socket>` and `ssh -S <socket> -O exit <jtaghost>`).
- JTAG over the tunnel is slower; the OpenOCD config already uses longer timeouts (30 s).

## Build

```shell
cd modules/neorv32/src/sw/gdb-example
make
```

## Debug

1. Start OpenOCD (keeps running, GDB server on port 3333):

   ```shell
   ./start-openocd.sh
   ```

   Before it calls `$QUARTUS/bin/openocd` with `modules/neorv32/openocd/neorv32_altera_vjtag.cfg`,
   the script
   - sets the cable's JTAG clock to 6 MHz (lost when the cable is replugged or `jtagd` restarts),
   - checks that `jtagconfig -n` lists the debug node; if not, it restarts a local `jtagd` (a hanging `jtagd`
     shows the devices without names and no nodes) and checks again,
   - reads the JTAG chain and declares a CoolRunner-II CPLD before/after the FPGA (e.g. pexp) to OpenOCD.

   Another cable (name as printed by `jtagconfig`): `CABLE='USB-BlasterII [1-6]' ./start-openocd.sh` (default).
   `./start-openocd.sh -h` lists all settings. Further arguments are passed to OpenOCD.

2. Connect GDB, load the program and stop at `main`:

   ```shell
   gdb-multiarch program.elf
   (gdb) set architecture riscv:rv32
   (gdb) target extended-remote :3333
   (gdb) monitor reset halt        # reset the CPU and halt at the reset vector
   (gdb) load                      # write program.elf into the Wishbone RAM (0x71000000)
   (gdb) break main
   (gdb) continue                  # stops in main (after the function prologue)
   (gdb) next                      # step line by line, "step" enters functions
   (gdb) break main.c:30           # start of the endless loop
   (gdb) continue                  # one loop iteration = one block of UART output
   (gdb) info locals
   ```

   `detach` leaves the CPU halted. `monitor resume` (or `continue` before detaching) lets it run again.

## Notes

- Memory map: the processor-internal IMEM is disabled. Code runs from the external Wishbone RAM
  (`neorv32_wb_ext_ram` in `neorv32_shell.vhd`, SDB name `NEORV32_RAM`, 0x71000000, reached via XBUS);
  data/stack use the internal DMEM at 0x80000000.
- The core has no hardware triggers, so all breakpoints are software breakpoints (`ebreak` written into the
  Wishbone RAM). This works because the code memory is RAM. Single stepping works as usual.
- `load` replaces the program in RAM only. After an FPGA reconfiguration the original init image runs again.
- While OpenOCD is connected, `jtagd` refuses to program the FPGA (`Error (209042): Application OpenOCD... is using
  the target device`). Stop OpenOCD (`monitor shutdown` or Ctrl-C), run `quartus_pgm`, then start OpenOCD again.
- How `load` reaches the RAM: the NEORV32 debug module has no system bus access, so OpenOCD halts the CPU and lets it
  execute store instructions from the debug program buffer (DTM -> DMI -> DM -> CPU `sw` -> XBUS -> Wishbone RAM).
- UART output (115200 8N1) appears on the board's USB serial port while the CPU runs and stops while it is halted.
- OpenOCD's `verify_image` needs a work area on the target and fails; read back with
  `monitor dump_image <file> 0x71000000 <size>` and compare with `program.bin` instead
  (the only differences are active software breakpoints).
