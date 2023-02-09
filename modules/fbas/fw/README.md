Location: **bel_projects/modules/fbas**
Components: WR switch, Pexaria (local) or SCU (remote)

## 1. Development setup

1.1. Minimal development environment with 2 Pexaria timing receivers (TR), and a WR switch (WRS) configured with dot-config_*_access_fbas

Two TRs must have the IP address: either per BOOTP or manually.

```
$ (sudo) eb-console dev/wbm0   # launch WR console
$ wrc# ip                      # check an IP address
$ wrc# ip set 192.168.131.30   # set an IP address, if it's missing
```
Feedback from a receiver node (RX node) to a sender node (TX node) is signalled via IO port: connect IO2 of TX node to IO1 of RX node with LEMO cable.

## 2. Build, install and debug a lm32 firmware

2.1. Build firmware binaries (.bin, *.elf) locally

Makefile is used to build the firmware binaries. Invoke a command given below:

```
$ make           # it will build firmware (*.elf, *.bin and other auxilary files)
$ make clean     # deletes all artifacts
```

2.2. Install a firmware binary to a target TR

Debug output messages output by each TR can be shown using **eb-console**, so launch it in additional terminal.

```
$ eb-console dev/wbm0     # 1st terminal
```

Afterwards, load the firmware binary to the target device using **eb-fwload** in the 2nd terminal.

```
$ eb-reset dev/wbm0 cpuhalt 0            # halt LM32
$ eb-fwload dev/wbm0 u 0x0 <firmware>    # load firmware to LM32 (with id 0 if multiple instances exist)
$ eb-reset dev/wbm0 cpureset 0           # (optional) restart the firmware
```

If everything works fine, you can see the expected debug output message on the 1st terminal.

```
Target BRG at base 0x84000000 0x84040000  entry 0
Target DEV at 0x84060000
fbas: CPU RAM External 0x 4060000, begin shared 0x00000500, command 0x04060508
sb_scan: app specific shared begin 0x10000500
common-fwlib: 720 bytes of shared mem are actually used
fbas0: SHARED_SET_NODETYPE 0x10000820
fbas0: SHARED_GET_NODETYPE 0x10000830
fbas0: SHARED_GET_TS1 0x1000092c

common-fwlib: ***** firmware fbas v000002 started from scratch *****

common-fwlib: 98 bytes of shared mem are actually used
Target DEV at 0x84000040
Target DEV at 0x840000c0
Target DEV at 0x80060300
Target DEV at 0x80060100
Target DEV at 0x84010000
fbas0: MAC=00:26:7b:00:04:da, IP=192.168.131.30
fbas0: pIOCtrl=84010000, pECAQ=840000c0
common-fwlib: changed to state 1
Target DEV at 0x84000040
Target DEV at 0x840000c0
Target DEV at 0x80060300
Target DEV at 0x80060100
Target DEV at 0x84010000
common-fwlib: changed to state 2
```

2.3. Debug the firmware

You have to enable the lowest __debug level__ (eg., 3) in **Makefile** so that all debug messages can be sent via UART.
With the EB tool **eb-console** you can see the debug output message sent by LM32 firmware.
For concrete example refer to section 3.

## 3. Tests

3.1. Measuring delay to forward MPS flag (FBAS event)

A simple test is performed to measure delays in forwarding FBAS events.

TX node is configured to act on 2 events: FBAS and IO.
On detection of FBAS event, the TX node generates a MPS flag and broadcasts it via WR network.
On IO event, the TX node calculates elapsed time to forward the MPS flag and outputs the calculation to the WR console.

Similarly, RX node is configured to act on timing event, such that it drives designated IO port.

Both nodes are connected to a WRS. Additionally their IO ports (RX:IO2, TX:IO1) are linked with the LEMO cable, which builds feedback channel.

The test procedure starts by injecting a timing event (FBAS event) to TX node manually with **saft-ctl** tool.
As response to FBAS event, TX node creates a MPS flag, broadcasts it as timing message periodically.

Since RX and TX nodes are connected to the same WRS, RX node will receive such timing messages and drives its IO1 port to signal reception of FBAS event.
This will cause IO event on TX node and elapsed time to forward the FBAS event is calculated.

All procedures of the test are listed in fbas/x86/setup.sh. To perform the test invoke steps in fbas/x86/Makefile.

## 4. Q&A

4.1. Compiler/linker errors/warnings

4.1.1. Shared libraries (libmpfr.so.4) not found

If cross-compiler cannot be run:

```
bel_projects/toolchain/bin/../libexec/gcc/lm32-elf/4.5.3/cc1: error while loading shared libraries: libmpfr.so.4: cannot open shared object file: No such file or directory
```

Follow the instructions given in 'bel_projects':

```
In Rocky 9: [link](https://github.com/GSI-CS-CO/bel_projects/tree/master/res/rocky-9)
In Ubuntu/Mint: [link](https://github.com/GSI-CS-CO/bel_projects#common-errors-and-warnings)
```

4.1.2. Linker returns a bunch of error messages with 'ebm_' prefix.

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

4.1.3. Compiler claims an undeclared identifier

```
fbastx.c: In function ‘initSharedMem’:
fbastx.c:106:71: error: ‘pCpuRamExternal’ undeclared (first use in this function)
```

Solution: check variable declaration (probably typo)

4.2. Run-time errors/warnings

4.2.1. Getting MAC/IP addresses fails

If the MAC/IP address detection fails, then debug output contains similar message given below:

```
fbastx: ERROR - init of EB master failed! 5
```

Solution:

[x] check network connection, if a timing receiver is connected to an WRS
[x] check link with **eb-console**
[x] check RVLAN status of WRS (**rvlan-status** on WRS)
[x] check if RADIUS server is reachable (**radtest** on WRS)

4.3. Other errors/warnings

4.3.1. WR console

4.3.1.1. Why warnings given below are printed repeatedly on WR console?

Warning: tx timestamp never became available
Warning: tx not terminated infinite mcr=0x51001410

Answer: unclear :-(
