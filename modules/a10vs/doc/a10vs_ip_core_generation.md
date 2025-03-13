# Using Arria 10 voltage sensor IP core in your HDL design

A list given below shows the EDA tools that are used to generate the voltage sensor IP core:

  - [quartus_gui]: "Quartus Prime Standard Edition v23.1"
  - [ip_param_editor_gui]: "IP Parameter Editor"
  - [ip_core_name]: "a10vs_ip"

## 1. Steps done in "[quartus_gui]".

Setup: Assignments -> Settings... -> IP Settings -> IP generation HDL preference: VHDL

1.1. Start "[quartus_gui]" and set "Device Family = Arria 10 (GX/SX/GT)" in the "IP Catalog" window.

1.2. Locate "Voltage Senser Intel FPGA IP" from the "Installed IP" -> "Library" -> "Configuration and Programming" list

1.3. Right-click on it and click on "Add version 23.1 ..." => it will open "[ip_param_editor_gui]".

## 2. Steps done in "[ip_param_editor_gui]"

2.1. Entity name and location of IP core

IP Variation
  - Entity name: [ip_core_name]
  - Save in folder: bel_projects/modules/a10vs/src/hdl/a10vs_ip

Target Device
  - Family: Arria 10
  - Device: 10AX027H2F34E2GS

There are 2 configurations available for the voltage sensor IP core.

2.2. Controller Configuration: core variant="Voltage controller with Avalon-MM sample storage"

2.3. Sample Storage Configuraiton: memory type="On-Chip Memory"

These settings can be saved as "default" preset in the "[ip_core_qsys_location]/ip/preset/default.qprs" file.

Generation of the HDL code is ready to be proceeded.

2.4. Click on "Generate HDL..."

  - synthesis: "VHDL" is chosen for HDL design files

2.5. Done "a10vs_ip" with 4 modules, 10 files:
  - a10vs_ip
  - a10vs_ip_altera_voltage_sensor_231_dihpyaa
  - a10vs_ip_altera_voltage_sensor_control
  - a10vs_ip_altera_voltage_sensor_sample_storage

Directory structure ("default" preset):

```
modules/a10vs/src/hdl/a10vs_ip/
├── a10vs_ip
│   ├── a10vs_ip_bb.v
│   ├── a10vs_ip.bsf
│   ├── a10vs_ip.cmp
│   ├── a10vs_ip.csv
│   ├── a10vs_ip.debuginfo
│   ├── a10vs_ip_generation_previous.rpt
│   ├── a10vs_ip_generation.rpt
│   ├── a10vs_ip.html
│   ├── a10vs_ip_inst.v
│   ├── a10vs_ip_inst.vhd
│   ├── a10vs_ip.ppf
│   ├── a10vs_ip.qip
│   ├── a10vs_ip.sip
│   ├── a10vs_ip.spd
│   ├── a10vs_ip.xml
│   ├── altera_voltage_sensor_231
│   │   ├── Manifest.py
│   │   ├── sim
│   │   │   └── a10vs_ip_altera_voltage_sensor_231_dihpyaa.v
│   │   └── synth
│   │       ├── a10vs_ip_altera_voltage_sensor_231_dihpyaa_cfg.v
│   │       ├── a10vs_ip_altera_voltage_sensor_231_dihpyaa.v
│   │       └── a10vs_ip_pkg.vhd                                 -- add to repo
│   ├── altera_voltage_sensor_control_231
│   │   ├── sim
│   │   │   ├── altera_voltage_sensor_control.sv
│   │   │   ├── voltage_sensor_avalon_control.sv
│   │   │   └── voltage_sensor_wrapper.sv
│   │   └── synth
│   │       ├── altera_voltage_sensor_control.sv
│   │       ├── voltage_sensor_avalon_control.sv
│   │       └── voltage_sensor_wrapper.sv
│   ├── altera_voltage_sensor_sample_store_231
│   │   ├── sim
│   │   │   ├── altera_voltage_sensor_sample_store_ram.sv
│   │   │   ├── altera_voltage_sensor_sample_store_register.sv
│   │   │   └── altera_voltage_sensor_sample_store.sv
│   │   └── synth
│   │       ├── altera_voltage_sensor_sample_store_ram.sv
│   │       ├── altera_voltage_sensor_sample_store_register.sv
│   │       └── altera_voltage_sensor_sample_store.sv
│   ├── sim                                                      -- simulation files
│   │   ├── a10vs_ip.vhd
│   │   ├── aldec
│   │   │   └── rivierapro_setup.tcl
│   │   ├── mentor
│   │   │   └── msim_setup.tcl
│   │   ├── synopsys
│   │   │   └── vcsmx
│   │   │       ├── synopsys_sim.setup
│   │   │       └── vcsmx_setup.sh
│   │   └── xcelium
│   │       ├── cds.lib
│   │       ├── cds_libs
│   │       │   ├── a10vs_ip_altera_voltage_sensor_231.cds.lib
│   │       │   ├── a10vs_ip_altera_voltage_sensor_control_231.cds.lib
│   │       │   └── a10vs_ip_altera_voltage_sensor_sample_store_231.cds.lib
│   │       ├── hdl.var
│   │       └── xcelium_setup.sh
│   └── synth
│       └── a10vs_ip.vhd                                         -- add to repo
├── a10vs_ip.qsys                                                -- add to repo
├── a10vs_ip.sopcinfo
└── altera_vs_ip_preset_0.qprs                                   -- preset file

where following files are required for target synthesis (pexarria10, scu4) later:
  - a10vs_ip/a10vs_ip/altera_voltage_sensor_231/Manifest.py            -- add later
  - a10vs_ip/a10vs_ip/altera_voltage_sensor_231/synth/a10vs_ip_pkg.vhd
  - a10vs_ip/a10vs_ip/synth/a10vs_ip.vhd
  - a10vs_ip/a10vs_ip.qsys

```

## 3. Implement a custom module in VHDL

The Wishbone bus is used as the main interconnection of HDL components within a Timing Receiver.
But the generated Intel IP core provides the Avalon-MM (or Avalon-ST) bus interface.
Hence, a dedicated HDL module is required to allow access to the Intel IP core.

The implemented top level module (a10vs) contains 2 components: Wishbone slave and generated IP core.

```
WB device crossbar  <=>  a10vs
                        ----------------------------------------------------------
                        | WB slave (a10vs_wb) <= Avalon-MM => IP core (a10vs_ip) |
                        ----------------------------------------------------------

```

All components implemented are shown in the 'modules/a10vs' folder:

```
├── Manifest.py
└── src
    └── hdl
        ├── a10vs_pkg.vhd
        ├── a10vs_tb.vhd
        ├── a10vs.tcl
        ├── a10vs.vhd
        └── a10vs_wb.vhd

where:

  - a10vs_pkg.vhd: package with the Wishbone slave
  - a10vs_tb.vhd:  testbench for simulation
  - a10vs.tcl:     TCL script to generate the IP core (qsys-generate a10vs_ip)
  - a10vs.vhd:     top level module
  - a10vs_wb.vhd:  Wishbone slave (interface to the IP core)
  - Manifest.py:   hdlmake synthesis
```

## 4. Integrate the a10vs module into the timing receiver top module (monster)

Update the monster module and target synthesis module:

  - modules/Manifest.py:
    - add "a10vs" in the "modules" list
  - modules/monster/monster.vhd:
    - add the "a10vs_pkg" package in the "work" library
    - extend the generic with "g_en_a10vs" (monster entity)
    - extend the device slave crossbar with "devs_a10vs" (dev_slaves type)
    - extend the sdb record with "devs_a10vs" (c_dev_layout_req_slaves)
    - add instance of the "a10vs" module in the wishbone slaves
  - modules/monster/monster_pkg.vhd:
    - add the "g_en_a10vs" generic and set it to "false"

  for 'pexarria10' target:
  - top/gsi_pexarria10/control/pci_control.vhd:
    - add the "g_en_a10vs" generic and set it to "true"
  - top/gsi_pexarria10/control/pci_control.tcl:
    - source ../../../modules/a10ts/src/hdl/a10vs.tcl

## 5. Set up Altera USB Blaster

__Notice__: these steps are done in Linux Mint 21.3 (kernel 5.15).

Altera USB Blaster device can be listed directly by 'lsusb'.

```
$ lsusb
...
Bus 001 Device 008: ID 09fb:6001 Altera Blaster
...
```

To enable non-root access to the USB Blaster device corresponding udev rules should be installed.
The 'bel_projects' repo contains a sample file with udev rules and it can be deployed into the '/etc/udev/rules.d/' folder under root permission [5.1]:

```
$ sudo cp bel_projects/doc/usbblaster/51-altera-usbblaster.rules /etc/udev/rules.d/
$ sudo udevadm control --reload   # re-load new rule file
```

In order to test the USB Blaster device plug it to a target FPGA device (ie., SCU3 or Pexarria10) and run 'jtagconfig'. The output should be looked as follows [5.2]:
```
$ jtagconfig                        # SCU3
1) USB-Blaster [1-8]
  025030DD   EP2AGX125(.|ES)

$ jtagconfig                        # Pexarria10
1) USB-Blaster [1-8]
  02EE30DD   10AX027E(1|2|3|4)/10AX027H1/..
```

If the USB device permissions are insufficient, the output might be:
```
No JTAG hardware available
```

If the USB device permissions are OK, but 'jtagd' is not running as root:
```
1) USB-Blaster [1-8]
  Unable to lock chain (Insufficient port permissions)
```

However, if permissions are OK, 'jtagd' runs as root, but access to the target FPGA device description fails:
```
1) USB-Blaster [1-8]
  025030DD
```

Links:
  - 5.1: [bel_projects: Installing the USB Blaster](https://github.com/GSI-CS-CO/bel_projects/fallout/doc/usbblaster/readme.md)
  - 5.2: [Using Intel/Altera USB Blaster on Debian Linux](https://fpgacpu.ca/fpga/debian.html)

## 6. Signal Tap logic analyzer

When the project is opened by [quartus_gui], start Signal Tap Logic Analyzer from 'Tools' menu.

### Configuration for 'auto_signaltap_0'

Signal Configuration:
  - clock: monster:main|core_clk_125m_local_i (signal tap: pre-synthesis)
  - sample depth: 128, RAM type: Auto
  - Nodes Allocated: Auto
  - Storage qualifier:
    - Type: Continuous

Node List (signal tap:pre-synthesis signals):
  - clk_i
  - slave_i.cyc  (trigger enable)
  - slave_i.stb  (trigger enable)
  - slave_i.we
  - slave_o.ack
  - vs_ctrl_csr_addr
  - vs_ctrl_csr_rd
  - vs_ctrl_csr_wr
  - vs_sample_csr_rd
  - vs_sample_csr_wr
  - slave_i.adr[31..0]
  - slave_i.dat[15..0]
  - slave_o.dat[15..0]
  - vs_ctrl_csr_rddata[15..0]
  - vs_sample_csr_addr[3..0]
  - vs_sample_csr_rdata[15..0]

The configuration can be stored in *.stp file.

### Compilation and programming

Once nodes/signals for inspection are selected, start the compilation by clicking on 'Start Compilation'. It will run following task to completion:
  - analysis & synthesis
  - place & route
  - generate bitstream
  - timing analysis

A corresponding *.sof file (ie., pexarria10.sof) will be created.
For programming the target device:
  - click on 'Browse Programming Files' to choose the *.sof file and
  - click on 'Programm Device' to flash

Assume that the target device (type: 10A027E) is mounted in the PCIe card and installed in a Linux host with EB tools. Use eb-{info|ls} tools to get the gateware information.
If the EB tools return an error, re-load the wishbone and PCIe drivers, or reboot the Linux host.

```
$ eb-ls dev/wbm0 | grep -i altera_voltage
13.34       0000000000000651:a1076000   40000c0 Altera_voltage_sens
```

### Analysis

Click on 'Run Analysis' to start the analysis. It will wait for trigger conditions get valid.

```
$ eb-read dev/wbm0 0x40000c0/4        # read the sample register 0
00000000

$ eb-read dev/wbm0 0x40000e0/4        # read the interrupt enable register
00000001                              # expected reset value

$ eb-read dev/wbm0 0x40000e8/4        # read the command register
00000000                              # expected reset value

$ eb-write dev/wbm0 0x40000e8/4 0x83  # set the cyclic mode and start the core operation
                                      # 0x83: MD[1:0]="01", MODE[1:0]="01", RUN="1"

$ eb-read dev/wbm0 0x40000e8/4        # read the command register
00000083                              # expected value
```

## A. Issues

### A.1 (open) Quartus cannot generate VHDL out of Qsys

TCL script to generate IP core: qsys-generate a10vs_ip
It seems that the TCL script reads the 'a10vs_ip/a10vs_ip.qsys' file, but it generates Verilog (not VHDL file) despite of the specific parameter in Qsys file:
```
<parameter name="hdlLanguage" value="VHDL" />
```

### A.2 (open) Quartus fitter crashes after taking whole RAM (out of memory)

If Signal Tap is configured and compiled then Quartus crashes during fitter is run. The local system has 16G physical RAM.

```
$ dmesg | tail -10
...
[710464.197384] oom-kill:constraint=CONSTRAINT_NONE,nodemask=(null),cpuset=/,mems_allowed=0,global_oom,task_memcg=/user.slice/user-1000.slice/user@1000.service/app.slice/app-org.gnome.Terminal.slice/vte-spawn-9403f599-4b68-4fc3-bf58-24a191909a3c.scope,task=quartus,pid=312439,uid=1000
[710464.197402] Out of memory: Killed process 312439 (quartus) total-vm:17477028kB, anon-rss:7150964kB, file-rss:0kB, shmem-rss:22560kB, UID:1000 pgtables:33936kB oom_score_adj:0
```

Check the physical [memory recommendations](https://www.intel.com/content/www/us/en/docs/programmable/683706/23-1/disk-space-and-memory-recommendations.html) for Quartus Prime Pro version 23.1.

## B. VHDL recall

signal:
  - interconnection wires that connect component instantiation ports together
  - declared in entity, architecture and package
  - assigned values are scheduled in the future (after delta delay)
  - signal <= value;

variable:
  - local/temporary storage in process statements and subprograms
  - assignment happens immediately after statement execution
  - variable := value;

process:
  - contains only sequential statements (if, case, loop)
  - but, itself it is a concurrent statement
  - has 2 parts: declaration section and statement part
  - types, variables, constants, subprograms and so on are declared in declaration section