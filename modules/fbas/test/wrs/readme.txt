management.txt                           - custom registry that is used to configure nwt0297m66

WRS configurations in the 'dot-configs' directory:
dot-config.default                       - default
dot-config.xenabay                       - modified default, with NTP and syslog server
dot-config_timing_mps_access             - dot-config for MPS access WRS in TTF
dot-config_timing_mps_access.rvlan       - modified timing_mps_access, where RVLAN is disabled -> used in Xenabay 'high_load_*' testbed
dot-config_timing_mps_access_xena        - dot-config for MPS access WRS in TTF (wri2/12 are reserved for RX/TX, usage with Xenabay 'broadcast_timing_msg' and 'high_load' testbeds)
dot-config_timing_mps_access_xena.rvlan  - modified timing_mps_access_xena, where RVLAN is disabled
dot-config_production_mps_access_ho      - dot-config for MPS access WRS in HO
