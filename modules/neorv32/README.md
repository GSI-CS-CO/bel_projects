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

To connect the JTAG adapter to the RISC-V core a “virtual JTAG” adapter is added from the IP library from Quartus.
As per the neorv32 documentation I tried OpenOCD, but haven’t gotten it to work yet. The target and the configuration file have to be modified. The GDB commands seem to be wrong and only work when adding an underscore.
