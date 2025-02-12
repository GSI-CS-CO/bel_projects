files = [
    # working, but without local_thr_mem "blm_aco_v1.0.vhd",
   "blm_aco_v1.1.vhd",
   "scu_diob_pkg.vhd",
   "scu_diob.sdc",
   "diob_debounce.vhd",
   "diob_sync.vhd",
   "aw_io_reg.vhd",
   "io_reg.vhd",
   "tag_ctrl.vhd",
   "tag_n.vhd",
   "config_status.vhd",
   "outpuls.vhd",
   "flanke.vhd",
   "addac_reg.vhd",
   "io_spi_dac_8420.vhd",
   "atr_comp_ctrl.vhd",
   "atr_cnt_n.vhd",
   "atr_puls_ctrl.vhd",
   "atr_puls_n.vhd",
   "atr_timeout.vhd",
   "fg901040.vhd",
   "in_reg.vhd",
   "BLM_watchdog_v1.0.vhd",
   "BLM_gate_timing_seq_v1.2.vhd",
   "up_down_counter.vhd",
   "Beam_Loss_check_v2.vhd", # current version 
  # "beam_loss_check_v3.vhd", # update
   "front_board_id_v0.vhd",
   "BLM_counter_pool_el.vhd",
   "BLM_out_el_m_v1.0.vhd",
   "IOBP_LED_ID_module_v1.0.vhd",
   "p_connector.vhd",
   "BLM_in_mux.vhd",
   "BLM_gate_el_v1.0.vhd",
   "BLM_ena_in_mux.vhd",
   "blm_24_9_9_9pll.vhd",
   "deglitcher.v",
   "BLM_cnt_pulse_former.vhd",
   "event_ctrl_el.vhd",
   "local_thr_mem.vhd",
   "bus_splitter.vhd",
    "th_ram.vhd",
    "l0ad_thr_fifo.vhd",
    "local_thr_box_2_1.vhd", # current version
   # "local_thr_box_v3.vhd",  # update
    "threshold_trigger_input.vhd",
    "clk_divider.vhd",
    "clk_divider_by_5.vhd"
]

modules = {
  "local" : [
    "../..",
  ]
}
