set platform "exploder5 +db[12] +wrex2"
source ../../autogen.tcl
source ../../../modules/build_id/build_id.tcl
source ../../../modules/nau8811/src/hdl/altera_pll/audio_pll.tcl
source ../../../ip_cores/general-cores/platform/altera/networks/arria5.tcl
source ../../../ip_cores/general-cores/platform/altera/wb_pcie/arria5.tcl
source ../../../modules/pll/arria5/arria5_pll.tcl
source ../../../ip_cores/wr-cores/platform/altera/wr_arria5_phy/wr_arria5_phy.tcl
source ../../common/arria5_legacy_flash_patch.tcl
source ../../common/arria5_serdes_lvds_patch.tcl
