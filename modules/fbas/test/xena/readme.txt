Configuration 'xenabay_ttf_broadcast_tim_msg.vmcfg' is used generate timing
messages (with the MPS flag). The testbeds are designed to work together with
following devices in TTF (rack BG2A.A9):
- SCUs: scuxl0396 (TX), scuxl0497 (RX)
- WRSs: nwt0297 (dot-config_timing_mps_access_xena)

1. broadcast_timing_msg: used to measure maximum data rate of background network traffic,
                         at which MPS signaling latency exceeds its upper bound of 1 ms.
                         Stream 0 is used to generate timing msgs with group and event ID = 0xFCA (not relevant to MPS)
                         tools: test_ttf_basic.ssh (run it at least for 60 sec)
                         nwt0297: dot-config_timing_mps_access_xena

2. high_load:            used to measure maximum reception rate of MPS RX node
                         Stream 1 is used to generate timing msgs with group and event ID = 0xFCB (MPS flag/event)
                         tools: test_ttf_high_load.sh (run it at least for 60 sec)
                         nwt0297: dot-config_timing_mps_access_xena

Configuration 'xenabay_ttf_send_capture_tim_msg' is mostly used to check
Xenabay itself (and 'M6SFP' modules) and can be used to capture timing messages
sent by scuxl0396.
