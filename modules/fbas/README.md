Location: **bel_projects/modules/fbas**

## 1. Introduction

In general, the purpose of all Machine Protection Systems (**MPS**) is to protect the beam production machines (UNILAC, SIS18, etc) from damage in dangerous situations.
One of these MPSs is the Fast Beam Abort System (**FBAS**) for the SIS100 machine.

According to the last-known Technical Concept ([pdf)](https://www-acc.gsi.de/wiki/pub/Timing/Intern/MPS/F-TC-C-03e_SIS100_Fast_Beam_Abort_System_Requirements_v1.0.pdf)), FBAS shall utilize the Timing network and Timing Receivers (**TR**) for signaling MPS events.
In MPS context, events are declared as changes of any machine protection condition.
For FBAS, such MPS events shall be distributed as timing events between the transmitter and receiver logic modules (SCUs).

The final goal of the project is to develop embedded modules for the FBAS event signaling system,  in form of:
   - firmware for embedded CPU (LM32) in TR
   - configuration of WR switches within the dedicated WR timing network
   - optional, API for user-space program (saftlib driver/plug-in)

## 2. Status

This project is in the development stage.

In order to probe FBAS event signaling and measure network performance a demo firmware for LM32 is implemented.
The work result is published in the Timing wiki site ([link](https://www-acc.gsi.de/wiki/bin/view/Timing/Intern/ProbingMPSEventSignalling)).

Directory structure:
   - fw: contains firmware source files and Makefile
   - include: header files
   - test: test artifacts (scripts, WRS configuration, DM schedule etc)
   - x86: diagnostic utility for host
   - rte: scripts for the legacy and yocto deployments on asl75x (update it :exclamation:)

## 3. Build, install and debug a LM32 firmware

### 3.1. Build firmware binary (.bin) locally

Makefile is used to build the firmware binaries (.bin, .elf). Invoke a command given below:

```
$ make           # it will build firmware, fbas*.scucontrol.bin
$ make clean     # deletes all artifacts
```

Starting from v1.5.1, only **fbas16.scucontrol.bin** is selected as valid firmware for tests.

### 3.2. Install a firmware binary to a target TR

Load the firmware binary to the target device using **eb-fwload**.

```
$ eb-reset dev/wbm0 cpuhalt 0            # halt LM32
$ eb-fwload dev/wbm0 u 0x0 <firmware>    # load firmware to LM32 (with id 0 if multiple instances of LM32 exist)
$ eb-reset dev/wbm0 cpureset 0           # (optional) restart the firmware
```

### 3.3. Debug the firmware

The lowest __debug level__ (eg., 3) should be set in **Makefile** so that all debug messages can be sent via UART.
Debug output messages output by each TR can be shown using **eb-console**, so launch it in additional terminal.

```
$ eb-console dev/wbm0     # in another terminal
```

A piece of the debug output message is given below as example.

```
fbas: CPU RAM: 0x20140000, common shared: 0x20140500
common-fwlib: 1716 bytes of shared mem are actually used

common-fwlib: ***** firmware fbas v010502 started from scratch *****
common-fwlib: fwlib_init, shared size [bytes], common part 464, total 1716
common-fwlib: cpuRamExternal 0x20140000,  cpuRamExternalData4EB 0x2014058c

fbas0: COMMON_CMD 0x10000508 (0x20140508)
fbas0: FBAS_BEGIN 0x10000900 (0x20140900)
fbas0: FBAS_SET_NODETYPE 0x10000908 (0x20140908)
fbas0: FBAS_GET_NODETYPE 0x10000918 (0x20140918)
fbas0: FBAS_GET_AVG 0x10000a58 (0x20140a58)
fbas0: FBAS_ECA_VLD 0x10000a80 (0x20140a80)
fbas0: FBAS_ECA_OVF 0x10000a84 (0x20140a84)
fbas0: FBAS_TX_MSG 0x10000a1c (0x20140a1c)
fbas0: FBAS_OLD_MSG 0x10000a20 (0x20140a20)
fbas0: FBAS_BAD_MSG 0x10000a24 (0x20140a24)
fbas0: FBAS_SENDERID 0x10000a88 (0x20140a88)
fbas0: FBAS_ACT_RATE 0x10000b90 (0x20140b90)
fbas0: FBAS_ERROR_FLAG 0x10000bb0 (0x20140bb0)
fbas0: FBAS_C2_MSG_BEGIN 0x10000800 (0x20140800)
fbas0: FBAS_C2_MSG_END 0x10000900 (0x20140900)
sbctl: SHARED_SET_SBSLAVES 0x1000090c
sbctl: SHARED_GET_SBSLAVES 0x1000091c
sbctl: SHARED_GET_SBSTDBEGIN 0x10000920
...

```

## 4. Tests

There are several test cases available in 'bel_projects/modules/fbas/test/tools'.

   - test the signaling network (test_ttf_nw_perf.sh, SCUs)
   - measure the maximum receiver rate (measure_ttf_rx_rate.sh, DM and SCUs)
   - test the ECA to SCU bus slave transfer (test_eca_scu_bus_slave.sh, DM and SCUs)
   - other

Basic test setup is built in TTF:
   - nwt0297m66: WR switch (*WRS*) configured with 'dot-config_timing_access_fbas'
   - scuxl0396: TX SCU
   - scuxl0497: RX SCU

If TTF test setup will be used, then it's recommended to run tests in any management master host (ie., tsl101).

~~All test-relevant artifacts can be deployed with a helper script ('bel_projects/modules/fbas/test/helpers/deploy_mngmt_host.sh') to the management master host (in the '$HOME/fbas_test' directory).~~

### 4.1. Running a test case

Assume, test artifacts are successfully deployed in target SCUs and DM.

In order to start a desired test case, switch to the '$HOME/fbas_test/tools' directory.

### 4.2. Test the signaling network

Here, signaling latency and some internal timing delay are measured:
   - ECA event handling delay: time period spent for fetching a MPS event from ECA  (measured by both TX and RX nodes)
   - transmitter delay: time period between fetching a MPS event and delivering to EB master (measured on TX node)
   - receiver delay: time period between fetcing a MPS event and storing it to a dedicated buffer (measured on RX node)
   - messaging delay: no meaning since the ECA to SCU bus slave transfer ~~time period between fetching a MPS event and driving the dedicated output signal (measured by RX node)~~
   - messaging latency: time period between generation of MPS event at TX node and generation of output signal on reception of corresponding timing message by RX node (measured on RX node)

TX SCU is configured to act on Power Converter (PC) event:
   - on detection of PC event, the TX SCU generates a corresponding timing messages (with FBAS Class 2 protocol, C2 protocol) and sends it via WR network

RX SCU is configured to act on timing events (with MPS flag) and do the corresponding ECA to SCU bus slave transfer. Here only 16-bit part of the timing message is transferred to a SCU bus slave, which provides a dedicated IO register.

The PC event on TX SCU is simulated by generating the PPS signal locally with **saft-ctl** tool.
The required ECA configuration for RX/TX SCUs is done also with **saft-ecpu-ctl** and **saft-io-ctl** tools.

All procedures of the test are scripted in each test/measurement scripts.

To start a test with basic setup, invoke a following command:

```
./test_ttf_nw_perf.sh -u root -e -y -v
```

To perform a test with other SCUs, specify the SCUs with -t (for TX) and -r (for RX) options:
```
./test_ttf_nw_perf.sh -u root -e -y -v -t scuxl0264 -t scuxl0305 -t scuxl0321 -t scuxl0329 -r scuxl0411
```

## 5. Q&A

### 5.1. Compiler/linker errors/warnings

#### 5.1.1. Shared libraries (libmpfr.so.4) not found

If cross-compiler cannot be run:

```
bel_projects/toolchain/bin/../libexec/gcc/lm32-elf/4.5.3/cc1: error while loading shared libraries: libmpfr.so.4: cannot open shared object file: No such file or directory
```

Follow the instructions given in 'bel_projects':

   - Ubuntu/Mint: [link](https://github.com/GSI-CS-CO/bel_projects#common-errors-and-warnings)
   - Rocky 9: [link](https://github.com/GSI-CS-CO/bel_projects/tree/master/res/rocky-9)

#### 5.1.2. Linker returns a bunch of error messages with 'ebm_' prefix.

```
/tmp/cchA6kkO.o: In function `fwlib_ebmInit':
common-fwlib.c:(.text.fwlib_ebmInit+0xd8): undefined reference to `ebm_init'
common-fwlib.c:(.text.fwlib_ebmInit+0xd8): relocation truncated to fit: R_LM32_CALL against undefined symbol `ebm_init'
```

Solution:

Edit Makefile so that missing components from the **lm32-include** module must be compiled. In this case a missing component is **ebm**.
So append **$(INCPATH)/ebm.c** to the existing target rule in **Makefile**.

```
$(TARGET).elf: $(PATHFW)/fbastx.c **$(INCPATH)/ebm.c** $(PATHFW)/../../common-libs/fw/common-fwlib.c
```

#### 5.1.3. Compiler claims an undeclared identifier

```
fbastx.c: In function ‘initSharedMem’:
fbastx.c:106:71: error: ‘pCpuRamExternal’ undeclared (first use in this function)
```

Solution: check variable declaration (probably typo)

### 5.2. Run-time errors/warnings

#### 5.2.1. Getting MAC/IP addresses fails

If the MAC/IP address detection fails, then debug output contains similar message given below:

```
fbastx: ERROR - init of EB master failed! 5
```

Solution:

   - [x] check network connection, if a timing receiver is connected to an WRS
   - [x] check link with **eb-console**
   - [x] check RVLAN status of WRS (**rvlan-status** on WRS)
   - [x] check if RADIUS server is reachable (**radtest** on WRS)

### 5.3. Other errors/warnings

#### 5.3.1. WR console

##### 5.3.1.1. Why warnings given below are printed repeatedly on WR console?

Warning: tx timestamp never became available
Warning: tx not terminated infinite mcr=0x51001410

Answer: unclear :-(
