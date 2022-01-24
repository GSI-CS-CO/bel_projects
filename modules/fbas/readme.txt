directory contents

- x86: tools or scripts invoked from x86 hosts (acopc017, tsl001)

-- invoked from 'acopc017':
    deploy_tsl001.sh  - deploy artifacts from 'acopc017' into 'tsl001'

-- invoked from 'tsl001':
    cp_scu_artifacts.sh   - copy LM32 firmware and other tools/scripts to a specified SCU
    system_info.sh        - show WR timing status, LM32 firmware and artifacts of a specified SCU
    test_ttf_basic.sh     - test timing message transmission (with MPS protocol) between
                            TX and RX nodes that are connected to nwt0297
    test_ttf_high_load.sh - test timing message transmission (with MPS protocol) by Xenabay
                         nwt0297 can be configured with (dot-config_timing_mps_access) or
                         w/o VLANs (dot-config.xenabay)

- scu: directory with SCU artifacts
        LM32 firmware
        shell script to be run locally on SCU to configure its FTRN

- v6.0.1: optional directory
        FTRN gateware dependent tools (eg., eb-fwload that is not included in ramdisk)

- wrs: directory with White Rabbit switch artifacts
        configurations for access switch
