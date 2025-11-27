directory contents

- dm: patterns for Datamaster

- helpers: helper scripts invoked from x86 host (acopc017, tsl101)
    deploy_mngmt_host.sh  - deploy test artifacts to 'tsl101'
    system_info.sh        - show WR timing status, LM32 firmware and artifacts of a specified SCU

- lm32: LM32 firmwares

- scu:  directory with SCU artifacts
    setup_local.sh        - shell script to be run locally on SCU to configure its FTRN
    v6.0.1                - (optional) FTRN gateware dependent artifacts (eg., eb-fwload that is not included in ramdisk)

- tools: test scripts invoked from x86 hosts (acopc017, tsl101)

-- prerequisites:
    all TRs must have their IP addresses

-- for test setup in TTF and BG2A.A9 (invoked from 'tsl101'):

    test_ttf_nw_perf.sh   - measure network performance using a small testbed with RX/TX SCUs and a WRS.
                            WRS must be configured with a corresponding dot-config (ie., dot-config_timing_mps_access).
                            LEMO cabling between the RX and TX SCUs is necessary (RX:B1 - TX:B2).
                            Applicable testbeds:
                                TTF:
                                    nwt0297: scuxl0497 (RX, wri2), scuxl0396 (TX, wri10)
                                BG2A.A9:
                                    nwt0470: scuxl0321 (RX, wri2), scuxl0264 (TX, wri10)
                                    nwt0471: scuxl0411 (RX, wri2), scuxl0329 (TX, wri10)
                                    nwt0472: scuxl0305 (RX, wri2), scuxl0339 (TX, wri10)
    test_ttf_basic.sh     - test timing message transmission (with MPS protocol) between
                            TX and RX nodes that are connected to nwt0297
                            It's intended to be used by Makefile.
                            For stand-alone run, launch it: ./test_ttf_basic.sh -u root
    test_ttf_high_load.sh - test timing message transmission (with MPS protocol) by Xenabay
                            nwt0297 can be configured with (dot-config_timing_mps_access) or
                            w/o VLANs (dot-config.xenabay)
    measure_ttf_rx_rate.sh - measure the data reception rate
                            results are reported in wiki (https://wiki.gsi.de/TOS/Timing/Intern/ProbingMPSEventSignalling)

-- for test setup in HO  (invoked from 'acopc017'):
    make <target>

- wrs: White Rabbit switch artifacts
    dot-config_*          - configurations for access switch

- xena: Xenabay traffic analyzer and generator configurations
    *.vmcfg               - XenaBay testbeds
    testbed_*             - directories with schedule, capture filter etc

Devices required for measure_ttf_rx_rate.sh (as of 27.5.2025)

- DM: tsl014 with pexaria5 devices
    - dev/wbm0 (pexaria32t)
    - dev/wbm1 (pexaria28t)
        - if eb-reset fails, then write "deadbeef" to the FPGA_RESET

- TR: SCUs with gateware v6.1.2 (or v6.2.1)
    - scuxl0396/497

- WRS: software v8.0 (or v7.0)
    - nwt0037m66: localmaster, ports wri15/16 are reserved for DMs
    - nwt0297m66: access_fbas, ports wri2/10 are reserved for TRs
        - wri2:  sucxl0497
        - wri10: sucxl0396

Deployment of test artifacts

- set up management host (ie., tsl101)
  - ./helpers/deploy_mngmt_host.sh $USER
