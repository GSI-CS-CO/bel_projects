class eca_tap (object):
  ifname = 'ctrl'

  adrDict = {
  'reset_owr'       : 0x00, # wo,  1 b, Resets ECA-Tap
  'clear_owr'       : 0x04, # wo,  4 b, b3: clear late count, b2: clear count/accu, b1: clear max, b0: clear min
  'capture_rw'      : 0x08, # rw,  1 b, Enable/Disable Capture
  'cnt_msg_get_1'   : 0x0c, # ro, 32 b, Message Count
  'cnt_msg_get_0'   : 0x10, # ro, 32 b, Message Count
  'diff_acc_get_1'  : 0x14, # ro, 32 b, Accumulated differences (dl - ts)
  'diff_acc_get_0'  : 0x18, # ro, 32 b, Accumulated differences (dl - ts)
  'diff_min_get_1'  : 0x1c, # ro, 32 b, Minimum difference
  'diff_min_get_0'  : 0x20, # ro, 32 b, Minimum difference
  'diff_max_get_1'  : 0x24, # ro, 32 b, Maximum difference
  'diff_max_get_0'  : 0x28, # ro, 32 b, Maximum difference
  'cnt_late_get'    : 0x2c, # ro, 32 b, Late Message Count
  'offset_late_rw'  : 0x30, # rw, 32 b, Offset on difference. Controls condition for Late Message Counter increment
  }

  adrDict_reverse = {
  0x00  : 'reset_owr', # wo,  1 b, Resets ECA-Tap
  0x04  : 'clear_owr', # wo,  4 b, b3: clear late count, b2: clear count/accu, b1: clear max, b0: clear min
  0x08  : 'capture_rw', # rw,  1 b, Enable/Disable Capture
  0x0c  : 'cnt_msg_get_1', # ro, 32 b, Message Count
  0x10  : 'cnt_msg_get_0', # ro, 32 b, Message Count
  0x14  : 'diff_acc_get_1', # ro, 32 b, Accumulated differences (dl - ts)
  0x18  : 'diff_acc_get_0', # ro, 32 b, Accumulated differences (dl - ts)
  0x1c  : 'diff_min_get_1', # ro, 32 b, Minimum difference
  0x20  : 'diff_min_get_0', # ro, 32 b, Minimum difference
  0x24  : 'diff_max_get_1', # ro, 32 b, Maximum difference
  0x28  : 'diff_max_get_0', # ro, 32 b, Maximum difference
  0x2c  : 'cnt_late_get', # ro, 32 b, Late Message Count
  0x30  : 'offset_late_rw', # rw, 32 b, Offset on difference. Controls condition for Late Message Counter increment
  }

  flagDict = {
  'reset'       : 'wp', # Resets ECA-Tap
  'clear'       : 'wp', # b3: clear late count, b2: clear count/accu, b1: clear max, b0: clear min
  'capture'     : 'rw', # Enable/Disable Capture
  'cnt_msg'     : 'rd', # Message Count
  'diff_acc'    : 'rd', # Accumulated differences (dl - ts)
  'diff_min'    : 'rd', # Minimum difference
  'diff_max'    : 'rd', # Maximum difference
  'cnt_late'    : 'rd', # Late Message Count
  'offset_late' : 'rw', # Offset on difference. Controls condition for Late Message Counter increment
  }

  valDict = {
  'reset'       : 0, # Resets ECA-Tap
  'clear'       : 0, # b3: clear late count, b2: clear count/accu, b1: clear max, b0: clear min
  'capture'     : 0, # Enable/Disable Capture
  'cnt_msg'     : 0, # Message Count
  'diff_acc'    : 0, # Accumulated differences (dl - ts)
  'diff_min'    : 0, # Minimum difference
  'diff_max'    : 0, # Maximum difference
  'cnt_late'    : 0, # Late Message Count
  'offset_late' : 0, # Offset on difference. Controls condition for Late Message Counter increment
  }
