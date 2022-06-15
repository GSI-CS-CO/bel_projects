ValkyrieManager configurations

These configurations are used to generate test traffic:
- MPS protocol in parameter field of a timing message
- MPS protocol structure: MAC, index, MPS flag

The testbeds in the configurations are designed to work with
following devices in TTF (rack BG2A.A9):
- SCUs: scuxl0396 (TX), scuxl0497 (RX)
- WRSs: nwt0297 with dot-config_timing_mps_access.rvlan (RVLAN disabled)

1. xenabay_ttf_send_capture_tim_msg.vmcfg
    - used to capture the network packets via port p000
    - used to send the custom test packet (MPS protocol) via port p001
        - check received message count by RX node
    - test tools
        - test_ttf_basic.sh

2. xenabay_ttf_test_beds.vmcfg
    - used to generate the test traffic (MPS protocol) via 6 ports (p021-p022, p040-p043)
        - check received message count by RX node
    - test tools
        - test_ttf_high_load.sh

-------------------------------------------------------

Obsolete configurations -> test streams must be updated

3. xenabay_ttf_broadcast_tim_msg.vmcfg
    - contains 2 testbeds
        - broadcast_timing_msg: used to measure maximum data rate of background network traffic,
                         at which MPS signalling latency exceeds its upper bound of 1 ms.
                         Stream 0 is used to generate timing msgs with group and event ID = 0xFCA (not relevant to MPS)
                         tools: test_ttf_basic.ssh (run it at least for 60 sec)
                         nwt0297: dot-config_timing_mps_access_xena.rvlan

        - high_load:     used to measure maximum reception rate of MPS RX node
                         Stream 1 is used to generate timing msgs with group and event ID = 0xFCB (MPS flag/event)
                         tools: test_ttf_high_load.sh (run it at least for 60 sec)
                         nwt0297: dot-config_timing_mps_access_xena.rvlan
