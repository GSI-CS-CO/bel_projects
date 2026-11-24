# NEORV32 Integration

NEORV32 evaluation and integration into `bel_projects`.

# Table of Contents

- [NEORV32 Integration](#neorv32-integration)
- [Table of Contents](#table-of-contents)
- [FAQ and Common Problems](#faq-and-common-problems)
  - [NEORV32 Sources](#neorv32-sources)
  - [Toolchain](#toolchain)
    - [Toolchain Naming](#toolchain-naming)
    - [Compiler Flags](#compiler-flags)
- [Simulation](#simulation)
- [Wishbone Adapter](#wishbone-adapter)
  - [Single Access Adapter](#single-access-adapter)
  - [Burst Mode Adapter (ECA message)](#burst-mode-adapter-eca-message)
- [JTAG](#jtag)

# FAQ and Common Problems

## NEORV32 Sources

Just run `make` to clone the NEORV32 repository.

## Toolchain

Mint/Ubuntu/Debian/Fedora/Red Hat

```bash
sudo apt install binutils-riscv64-unknown-elf gcc-riscv64-unknown-elf picolibc-riscv64-unknown-elf
```

### Toolchain Naming

Depending on your Linux distribution, a different toolchain may be installed.

- Bare metal toolchain: riscv64-unknown-elf-gcc
- "Linux" userland toolchain (adds CRT0, stdlib and other infos by default): riscv64-linux-gnu-gcc

### Compiler Flags

| Flag | Explanation |
| - | - |
| -nostartfiles       | Do not link startup files (CRT0, ...). Avoids automatic insertion of startup code. |
| -nostdlib           | Do not link the standard C library and others startup files. Bare-metal without libc (not used here). |
| -march=rv32i        | Target architecture: RISC-V 32-bit base integer instruction set (RV32I). † |
| -mabi=ilp32         | Target ABI (Application Binary Interface): integer, long, pointer are 32-bit wide. RV32I convention. † |
| -Wl,--build-id=none | Passes --build-id=none to the linker (ld), disabling generation of .note.gnu.build-id section. Without this flag the MIF file will be broken. |
| -lc                 | Add libc.a (printf, malloc, memcpy, exit, open, close, ...) |
| -lgcc               | Add libgcc.a (division in assembler, C++ exception handling, ... ) |
| -L<pfad>            | Search path for libraries.|
| -Wl,--gc-sections   | Remove unused sections (linker flag). |
| -ffunction-sections | Remove unused functions (small bin/MIF). |
| -fdata-sections     | Remove unused data (small bin/MIF). |

† Needed when riscv64-XYZ is used.

# Simulation
From the `bel_projects` root, run both simulation tests with GHDL:

```bash
make test_neorv32 NO_QUARTUS=yes
```

`NO_QUARTUS=yes` uses the simulation libraries from the repository instead of `/eda/sim_lib`. Both Slim and Xbar tests passed with exit code 0.

```
cd sim
make
make view # this will open GTKWave
```

# Wishbone Adapter

The XBUS interface of the NEORV32 project does not fully conform to the wishbone standard, especially the pipelined mode. To make it compatible an adapter was implemented in the shell (wrapper).

## Single Access Adapter

In single access mode the adapter only handles stalls from the crossbar by registering the bus signals until the stall is over.

## Burst Mode Adapter (ECA message)

At least to enable insertions into the ECA queue a block transfer/pipelined mode is necessary. As there exists no stall signal from the XBUS interface the messages during a stall need to be registered in the adapter. The FIFO is sized to be able to fit the full ECA message.

# JTAG

The on-chip debugger is reached through the Altera SLD hub (virtual JTAG), so the same USB-Blaster cable is used for
programming, SignalTap and debugging (see [neorv32 discussion #1587](https://github.com/stnolting/neorv32/discussions/1587)).

- Gateware: `src/hdl/neorv32_debug_dtm_altera_sld.vhd` replaces the default `neorv32_debug_dtm` (selected in `Manifest.py`).
  It instantiates its own `sld_virtual_jtag` (SLD node id `0x00406E00`), the DTM `jtag_*` ports are unused,
  so targets should set `g_en_virtual_jtag => false`.
- OpenOCD: the Altera fork shipped with Quartus 23 (`$QUARTUS/bin/openocd`, adapter `aji_client`)
  with `openocd/neorv32_altera_vjtag.cfg`. It connects to the Quartus JTAG server (`jtagd`), which may run remotely.
  `src/sw/gdb-example/start-openocd.sh` sets the JTAG clock, checks the debug node, detects a CPLD in the chain
  and starts OpenOCD (see `src/sw/gdb-example/README.md`).
- Tested on pexp (`pexp-neorv32`, CPLD in the chain: add `-c 'set PRE_TAPS {{cpld 8 0x06e5d093}}'`,
  SignalTap can stay enabled) and on a pexarria5 test build.
- `jtagconfig -n` must list `Node 00406E00 (110:8) #0`. If the node is missing, lower the JTAG clock, e.g.
  `jtagconfig --setparam "USB-BlasterII [1-6]" JtagClock 6M` (24 MHz did not work on a pexarria5 via Promo12 adapter).

```
openocd -c 'set CABLE {USB-BlasterII on 127.0.0.1:13090 [1-6]}' -f modules/neorv32/openocd/neorv32_altera_vjtag.cfg
# then: telnet localhost 4444  (halt, reg pc, mdw 0x71000000 4, resume)  or  gdb: target extended-remote :3333
```
