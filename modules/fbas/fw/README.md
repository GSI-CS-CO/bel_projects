Location: **bel_projects/modules/fbas**
Components: WRS, Pexaria (local), SCU (remote)

## Development setup

1. Minimal development environment with 2 Pexaria timing receivers, and a WRS (dot-config_integration_access_fbas/production_access_fbas)

Two TRs are required: one is in the **PTP slave** mode (by default) and other is in the **PTP master** mode.

```
$ (sudo) eb-console dev/wbm0   # check the PTP mode of 1st Pexaria
$ wrc# mode                    # should return 'running; e2e slave'
```

```
$ (sudo) eb-console dev/wbm0   # launch WR console
$ wrc# ip                      # check an IP address
$ wrc# ip set 192.168.131.30   # set an IP address, if it's missing
```

From now on the LM32 firmware for FBAS can get the NIC info (MAC/IP addresses):

```
common-fwlib: ***** firmware fbastx v000002 started from scratch *****
...
fbastx: MAC=00:26:7b:00:04:da, IP=192.168.131.30
```

Let the 2nd Pexaria run as a PTP master

```
$ (sudo) eb-console dev/wbm1   # set the PTP master mode in 2nd Pexaria
$ wrc# mode master             # should return 'PTP stop Locking PLL...' if it could be able to lock PLL and get master
$ wrc# mode                    # should return 'running; e2e master'
```

FIX: PTP cannot be synchronized when 2 timing receivers (PTP master and slave) are connected directly: link is down!

## Build, install and debug

1. Build firmware binaries (.bin, *.elf) locally

Makefile is used to build the firmware binaries. Invoke a command given below:

```
$ make           # it will build firmware (*.elf, *.bin and other auxilary files)
$ make clean     # deletes all artifacts
```

2. Install a firmware binary to a target timing receiver (SCU)

If firmware sends out any debug message and you want to see it, then you need to 2 terminals.

Debug output messages can be shown with **eb-console**, so launch it in 1st terminal.

```
$ (sudo) eb-console dev/wbm0     # 1st terminal
```

Afterwards, load the firmware binary to the target device using **eb-fwload** in the 2nd terminal.

```
$ (sudo) eb-reset dev/wbm0 cpuhalt 0            # halt LM32
$ (sudo) eb-fwload dev/wbm0 u 0x0 fbastx.bin    # load firmware to LM32 (with id 0 if multiple instances exist)
$ (sudo) eb-reset dev/wbm0 cpureset 0           # (optional) restart the firmware
```

If everything works fine, you can see the expected debug output message on the 1st terminal.

```
Target BRG at base 0x84000000 0x84040000  entry 0
Target DEV at 0x84060000
wr-unipz: CPU RAM External 0x 4060000, begin shared 0x00000500
wr-unipz: COMMON_SHARED_BEGIN 0x10000500
wr-unipz: used size of shared mem is 0 words (uint32_t), begin 10000500, end 100004fc

common-fwlib: ***** firmware fbastx v000002 started from scratch *****

common-fwlib: 98 bytes of shared mem are actually used
common-fwlib: changed to state 1
Target DEV at 0x84000040
Target DEV at 0x840000c0
Target DEV at 0x80060300
Target DEV at 0x80060100
Target DEV at 0x84010000
common-fwlib: changed to state 2
```

3. Debug the firmware

You have to enable the lowest __debug level__ (eg., 3) in **Makefile** so that all debug messages can be sent via UART.
With the EB tool so-called **eb-console** you can see the debug output message sent by LM32 firmware.
For concrete example refer to section 3.

## Q&A

1. Compiler/linker errors/warnings

1.1. Linker returns a bunch of error messages with 'ebm_' prefix.

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

1.2. Compiler claims an undeclared identifier

```
fbastx.c: In function ‘initSharedMem’:
fbastx.c:106:71: error: ‘pCpuRamExternal’ undeclared (first use in this function)
```

Solution: check variable declaration (probably typo)

2. Run-time errors/warnings

2.1. Getting MAC/IP addresses fails

If the MAC/IP address detection fails, then debug output contains similar message given below:

```
fbastx: ERROR - init of EB master failed! 5
```

Solution:

[x] check network connection, if a timing receiver is connected to an WRS
[x] check link (**eb-console** on SCU)
[x] check RVLAN status of WRS (**rvlan-status** on WRS)
[x] check if RADIUS server is reachable (**radtest** on WRS)

3. Other errors/warnings

3.1. WR console

3.1.1. Why warnings given below are printed repeatedly on WR console?

Warning: tx timestamp never became available
Warning: tx not terminated infinite mcr=0x51001410

Answer: unclear :-(
