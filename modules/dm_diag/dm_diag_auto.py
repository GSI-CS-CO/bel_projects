class dm_diag (object):
  ifname = 'ctrl'

  adrDict = {
  'reset_owr'                       : 0x000, # wo,  1 b, Resets/clears the diagnostic
  'enable_rw'                       : 0x004, # rw,  1 b, Enables/disables update. Default is enabled
  'time_observation_interval_rw_1'  : 0x008, # rw, 32 b, TAI time observation interval in ns
  'time_observation_interval_rw_0'  : 0x00c, # rw, 32 b, TAI time observation interval in ns
  'time_dif_pos_get_1'              : 0x010, # ro, 32 b, Observed max pos. ECA time difference in ns between ref clock ticks
  'time_dif_pos_get_0'              : 0x014, # ro, 32 b, Observed max pos. ECA time difference in ns between ref clock ticks
  'time_dif_pos_ts_get_1'           : 0x018, # ro, 32 b, (approximate) timestamp of last pos dif update
  'time_dif_pos_ts_get_0'           : 0x01c, # ro, 32 b, (approximate) timestamp of last pos dif update
  'time_dif_neg_get_1'              : 0x020, # ro, 32 b, Observed max neg. ECA time difference in ns between ref clock ticks
  'time_dif_neg_get_0'              : 0x024, # ro, 32 b, Observed max neg. ECA time difference in ns between ref clock ticks
  'time_dif_neg_ts_get_1'           : 0x028, # ro, 32 b, (approximate) timestamp of last neg dif update
  'time_dif_neg_ts_get_0'           : 0x02c, # ro, 32 b, (approximate) timestamp of last neg dif update
  'wr_lock_cnt_get_1'               : 0x030, # ro, 32 b, cnt of wr lock bit going from low to high
  'wr_lock_cnt_get_0'               : 0x034, # ro, 32 b, cnt of wr lock bit going from low to high
  'wr_lock_loss_last_ts_get_1'      : 0x038, # ro, 32 b, timestamp of last wr lock loss
  'wr_lock_loss_last_ts_get_0'      : 0x03c, # ro, 32 b, timestamp of last wr lock loss
  'wr_lock_acqu_last_ts_get_1'      : 0x040, # ro, 32 b, timestamp of last wr lock acquired
  'wr_lock_acqu_last_ts_get_0'      : 0x044, # ro, 32 b, timestamp of last wr lock acquired
  'stall_observation_interval_rw'   : 0x048, # rw, 32 b, Stall observation interval in cycles
  'stall_stat_select_rw'            : 0x04c, # rw,  8 b, Page selector register for Stall observers
  'stall_streak_max_get'            : 0x100, # ro, 32 b, Observed max continuous stall in cycles
  'stall_cnt_get'                   : 0x104, # ro, 32 b, Stall time within observation interval in cycles
  'stall_max_ts_get_1'              : 0x108, # ro, 32 b, Timestamp of last max update
  'stall_max_ts_get_0'              : 0x10c, # ro, 32 b, Timestamp of last max update
  }

  adrDict_reverse = {
  0x000 : 'reset_owr', # wo,  1 b, Resets/clears the diagnostic
  0x004 : 'enable_rw', # rw,  1 b, Enables/disables update. Default is enabled
  0x008 : 'time_observation_interval_rw_1', # rw, 32 b, TAI time observation interval in ns
  0x00c : 'time_observation_interval_rw_0', # rw, 32 b, TAI time observation interval in ns
  0x010 : 'time_dif_pos_get_1', # ro, 32 b, Observed max pos. ECA time difference in ns between ref clock ticks
  0x014 : 'time_dif_pos_get_0', # ro, 32 b, Observed max pos. ECA time difference in ns between ref clock ticks
  0x018 : 'time_dif_pos_ts_get_1', # ro, 32 b, (approximate) timestamp of last pos dif update
  0x01c : 'time_dif_pos_ts_get_0', # ro, 32 b, (approximate) timestamp of last pos dif update
  0x020 : 'time_dif_neg_get_1', # ro, 32 b, Observed max neg. ECA time difference in ns between ref clock ticks
  0x024 : 'time_dif_neg_get_0', # ro, 32 b, Observed max neg. ECA time difference in ns between ref clock ticks
  0x028 : 'time_dif_neg_ts_get_1', # ro, 32 b, (approximate) timestamp of last neg dif update
  0x02c : 'time_dif_neg_ts_get_0', # ro, 32 b, (approximate) timestamp of last neg dif update
  0x030 : 'wr_lock_cnt_get_1', # ro, 32 b, cnt of wr lock bit going from low to high
  0x034 : 'wr_lock_cnt_get_0', # ro, 32 b, cnt of wr lock bit going from low to high
  0x038 : 'wr_lock_loss_last_ts_get_1', # ro, 32 b, timestamp of last wr lock loss
  0x03c : 'wr_lock_loss_last_ts_get_0', # ro, 32 b, timestamp of last wr lock loss
  0x040 : 'wr_lock_acqu_last_ts_get_1', # ro, 32 b, timestamp of last wr lock acquired
  0x044 : 'wr_lock_acqu_last_ts_get_0', # ro, 32 b, timestamp of last wr lock acquired
  0x048 : 'stall_observation_interval_rw', # rw, 32 b, Stall observation interval in cycles
  0x04c : 'stall_stat_select_rw', # rw,  8 b, Page selector register for Stall observers
  0x100 : 'stall_streak_max_get', # ro, 32 b, Observed max continuous stall in cycles
  0x104 : 'stall_cnt_get', # ro, 32 b, Stall time within observation interval in cycles
  0x108 : 'stall_max_ts_get_1', # ro, 32 b, Timestamp of last max update
  0x10c : 'stall_max_ts_get_0', # ro, 32 b, Timestamp of last max update
  }

  flagDict = {
  'reset'                       : 'wp', # Resets/clears the diagnostic
  'enable'                      : 'rw', # Enables/disables update. Default is enabled
  'time_observation_interval'   : 'rw', # TAI time observation interval in ns
  'time_dif_pos'                : 'rd', # Observed max pos. ECA time difference in ns between ref clock ticks
  'time_dif_pos_ts'             : 'rd', # (approximate) timestamp of last pos dif update
  'time_dif_neg'                : 'rd', # Observed max neg. ECA time difference in ns between ref clock ticks
  'time_dif_neg_ts'             : 'rd', # (approximate) timestamp of last neg dif update
  'wr_lock_cnt'                 : 'rd', # cnt of wr lock bit going from low to high
  'wr_lock_loss_last_ts'        : 'rd', # timestamp of last wr lock loss
  'wr_lock_acqu_last_ts'        : 'rd', # timestamp of last wr lock acquired
  'stall_observation_interval'  : 'rw', # Stall observation interval in cycles
  'stall_stat_select'           : 'rwf', # Page selector register for Stall observers
  'stall_streak_max'            : 'rdm', # Observed max continuous stall in cycles
  'stall_cnt'                   : 'rdm', # Stall time within observation interval in cycles
  'stall_max_ts'                : 'rdm', # Timestamp of last max update
  }

  valDict = {
  'reset'                       : 0, # Resets/clears the diagnostic
  'enable'                      : 1, # Enables/disables update. Default is enabled
  'time_observation_interval'   : 0, # TAI time observation interval in ns
  'time_dif_pos'                : 0, # Observed max pos. ECA time difference in ns between ref clock ticks
  'time_dif_pos_ts'             : 0, # (approximate) timestamp of last pos dif update
  'time_dif_neg'                : 0, # Observed max neg. ECA time difference in ns between ref clock ticks
  'time_dif_neg_ts'             : 0, # (approximate) timestamp of last neg dif update
  'wr_lock_cnt'                 : 0, # cnt of wr lock bit going from low to high
  'wr_lock_loss_last_ts'        : 0, # timestamp of last wr lock loss
  'wr_lock_acqu_last_ts'        : 0, # timestamp of last wr lock acquired
  'stall_observation_interval'  : 0, # Stall observation interval in cycles
  'stall_stat_select'           : 0, # Page selector register for Stall observers
  'stall_streak_max'            : 0, # Observed max continuous stall in cycles
  'stall_cnt'                   : 0, # Stall time within observation interval in cycles
  'stall_max_ts'                : 0, # Timestamp of last max update
  }
