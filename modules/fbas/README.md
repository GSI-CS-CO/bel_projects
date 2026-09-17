Location: **bel_projects/modules/fbas**

## 1. Introduction

In general, the purpose of all Machine Protection Systems (**MPS**) is to protect the beam production machines (UNILAC, SIS18, etc) from damage in dangerous situations.
One of these MPSs is the Fast Beam Abort System (**FBAS**) for the SIS100 machine.

According to the Technical Concept ([pdf)](https://www-acc.gsi.de/wiki/pub/Timing/Intern/MPS/F-TC-C-03e_SIS100_Fast_Beam_Abort_System_Requirements_v1.0.pdf)), FBAS shall utilize the Timing network and Timing Receivers (**TR**) for signaling MPS events.
In MPS context, events are declared as changes of any machine protection condition.
For FBAS, such MPS events shall be distributed as timing events between the transmitter and receiver logic modules (SCUs).

The final goal of the project is to develop embedded modules for the FBAS event signaling system,  in form of:
   - firmware for embedded CPU (LM32) in TR
   - API for user-space program (saftlib driver/plug-in)
   - WR timing network

## 2. Status

This project is in the development stage.

In order to probe FBAS event signaling and measure network performance a demo firmware for LM32 is implemented.
The work result is published in the Timing wiki site ([link](https://www-acc.gsi.de/wiki/bin/view/Timing/Intern/ProbingMPSEventSignalling)).

Directory structure:
   - fw: contains firmware source files and Makefile
   - include: header files
   - test: test artifacts (scripts, WRS configuration, DM schedule etc)

## 3. Build, install and debug a LM32 firmware

### 3.1. Build firmware binary (.bin) locally

Makefile is used to build the firmware binaries (.bin, .elf). Invoke a command given below:

```
$ make           # it will build firmware, fbas*.scucontrol.bin
$ make clean     # deletes all artifacts
```

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
Target BRG at base 0xa0000000 0xa0100000  entry 0
Target DEV at 0xa0140000
Target DEV at 0xa0180000
fbas: CPU RAM External 0x20140000, begin shared 0x00000500, command 0x20140508
fbas: app specific shared begin 0x10000500
fbas0: SHARED_SET_NODETYPE 0x10000820
fbas0: SHARED_GET_NODETYPE 0x10000830
fbas0: SHARED_GET_TS1 0x10000938
fbas0: SHARED_GET_CNT 0x10000934
fbas0: SHARED_CNT_VAL 0x10000990
fbas0: SHARED_CNT_OVF 0x10000994
fbas0: SHARED_SENDERID 0x10000998
common-fwlib: 788 bytes of shared mem are actually used

common-fwlib: ***** firmware fbas v000002 started from scratch *****
common-fwlib: fwlib_init, shared size [bytes], common part 392, total 788
common-fwlib: cpuRamExternal 0x20140000,  cpuRamExternalData4EB 0x20140544
...

```

## 4. Tests

There are several test cases available in 'bel_projects/modules/fbas/test/tools'.

   - measure signaling network performance (test_ttf_nw_perf.sh)
   - test timing message transmission (test_ttf_basic.sh)
   - other

Basic test setup is built in TTF:
   - nwt0297m66: WR switch (*WRS*) configured with 'dot-config_timing_access_fbas'
   - scuxl0396: TX SCU
   - scuxl0497: RX SCU

If TTF test setup will be used, then it's recommended to run tests in any management master host (ie., tsl101).
All test-relevant artifacts can be deployed with a helper script ('bel_projects/modules/fbas/test/deploy_mngmt_host.sh') to the management master host (in the '$HOME/fbas_test' directory).

### 4.1. Running a test case

All TRs must have their IP address before testing: either per BOOTP or manually.

```
$ (sudo) eb-console dev/wbm0   # launch WR console
$ wrc# ip                      # check an IP address
$ wrc# ip set 192.168.131.30   # set an IP address, if it's missing
```

Assume, test artifacts are successfully deployed in 'tsl101'.

In order to start a desired test case, switch to the '$HOME/fbas_test/tools' directory.

### 4.2. Measure the signaling network performance

Here, signaling latency and some internal timing delay are measured:
   - event handling delay: time period between generation of MPS event at TX node and storing of corresponding MPS flag at TX node (measured by TX node)
   - messaging delay (alias one-way): time period between transmission of timing message (with MPS event) by TX node and handling of the same timing message by RX node (measured by RX node)
   - transmission delay: time period between detection of MPS event by TX node and generation of output signal on reception of corresponding timing message by RX node (measured on TX node by using feedback from RX node)
   - signaling latency: time period between generation of MPS event at TX node and generation of output signal on reception of corresponding timing message by RX node (measured on TX node by using feedback from RX node)

TX SCU is configured to act on 2 events: FBAS and feedback (IO).
   - on detection of FBAS event, the TX SCU generates a MPS flag and broadcasts it via WR network.
   - on feedback event, the TX node calculates the elapsed time to signal FBAS event.

Similarly, RX SCU is configured to act on timing event (with MPS flag) and drives its output port to signal the FBAS event reception.
The feedback channel from RX SCU to TX SCU is made of the LEMO cable connection: RX:IO1 to TX:IO2.

The FBAS event is simulated by injecting a timing event TX SCU locally with **saft-ctl** tool.
The required event configuration for RX/TX SCUs is done also with **saft-ecpu-ctl** and **saft-io-ctl** tools.

All procedures of the test are scripted in 'test_ttf_nw_perf.sh'.
To start a test with basic setup, invoke below command (in $HOME/fbas_test/tools):

```
./test_ttf_nw_perf.sh -v
```

Assume, another setup is built with two TX SCUs (TX1=scuxl0339 and TX2=scuxl0305) and a RX SCU (scuxl0411).
The IO connection between RX and TX nodes via LEMO cable: RX:IO1 <===> TX1:IO2, RX:IO2 <===> TX2:IO2.
For 'test_ttf_nw_perf.sh' the **order of TX nodes** should reflect the IO connection, otherwise the test run will return dubious measurement results:

```
./test_ttf_nw_perf.sh -v -y -t scuxl0329 -t scuxl0305 -r scuxl0411
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
