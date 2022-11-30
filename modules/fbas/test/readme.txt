directory contents

- dm: patterns for Datamaster

- helpers: helper scripts invoked from x86 host (acopc017, tsl101)
    deploy_mngmt_host.sh  - deploy test artifacts to 'tsl101'
    cp_scu_artifacts.sh   - copy LM32 firmware and other tools/scripts to a specified SCU
    system_info.sh        - show WR timing status, LM32 firmware and artifacts of a specified SCU

- lm32: LM32 firmwares

- scu:  directory with SCU artifacts
    setup_local.sh        - shell script to be run locally on SCU to configure its FTRN
    v6.0.1                - (optional) FTRN gateware dependent artifacts (eg., eb-fwload that is not included in ramdisk)

- tools: test scripts invoked from x86 hosts (acopc017, tsl001)

-- for test setup in TTF (invoked from 'tsl001'):
    test_ttf_basic.sh     - test timing message transmission (with MPS protocol) between
                            TX and RX nodes that are connected to nwt0297
    test_ttf_high_load.sh - test timing message transmission (with MPS protocol) by Xenabay
                            nwt0297 can be configured with (dot-config_timing_mps_access) or
                            w/o VLANs (dot-config.xenabay)
    test_ttf_nw_perf.sh   - measure network performance

-- for test setup in HO  (invoked from 'acopc017'):
    make <target>

- wrs: White Rabbit switch artifacts
    dot-config_*          - configurations for access switch

- xena: Xenabay traffic analyzer and generator configurations
    *.vmcfg               - XenaBay testbeds
    testbed_*             - directories with schedule, capture filter etc

Deployment of test artifacts

- set up management host (ie., tsl101)
  - ./helpers/deploy_mngmt_host.sh $USER

- set up target SCUs for the FBAS test (ie., scuxl0396)
  - ./helpers/cp_scu_artifacts.sh -s scuxl0396
