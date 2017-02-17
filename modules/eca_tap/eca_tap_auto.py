class eca_tap (object):
  ifname = 'ctrl'

  adrDict = {
  'reset_owr'       : 0x00, # wo,  1 b, Resets ECA-Tap
  'clear_owr'       : 0x04, # wo,  3 b, b2: clear count/accu, b1: clear max, b0: clear min
  'cnt_msg_get_0'   : 0x08, # ro, 32 b, Message Count
  'cnt_msg_get_1'   : 0x0c, # ro, 32 b, Message Count
  'diff_acc_get_0'  : 0x10, # ro, 32 b, Accumulated differences (dl - ts)
  'diff_acc_get_1'  : 0x14, # ro, 32 b, Accumulated differences (dl - ts)
  'diff_min_get_0'  : 0x18, # ro, 32 b, Minimum difference
  'diff_min_get_1'  : 0x1c, # ro, 32 b, Minimum difference
  'diff_max_get_0'  : 0x20, # ro, 32 b, Maximum difference
  'diff_max_get_1'  : 0x24, # ro, 32 b, Maximum difference
  }

  adrDict_reverse = {
  0x00  : 'reset_owr', # wo,  1 b, Resets ECA-Tap
  0x04  : 'clear_owr', # wo,  3 b, b2: clear count/accu, b1: clear max, b0: clear min
  0x08  : 'cnt_msg_get_0', # ro, 32 b, Message Count
  0x0c  : 'cnt_msg_get_1', # ro, 32 b, Message Count
  0x10  : 'diff_acc_get_0', # ro, 32 b, Accumulated differences (dl - ts)
  0x14  : 'diff_acc_get_1', # ro, 32 b, Accumulated differences (dl - ts)
  0x18  : 'diff_min_get_0', # ro, 32 b, Minimum difference
  0x1c  : 'diff_min_get_1', # ro, 32 b, Minimum difference
  0x20  : 'diff_max_get_0', # ro, 32 b, Maximum difference
  0x24  : 'diff_max_get_1', # ro, 32 b, Maximum difference
  }

  flagDict = {
  'reset'     : ['wp', 1], # Resets ECA-Tap
  'clear'     : ['wp', 3], # b2: clear count/accu, b1: clear max, b0: clear min
  'cnt_msg'   : ['rd', 64], # Message Count
  'diff_acc'  : ['rd', 64], # Accumulated differences (dl - ts)
  'diff_min'  : ['rd', 64], # Minimum difference
  'diff_max'  : ['rd', 64], # Maximum difference
  }

  valDict = {
  'reset'     : 0, # Resets ECA-Tap
  'clear'     : 0, # b2: clear count/accu, b1: clear max, b0: clear min
  'cnt_msg'   : 0, # Message Count
  'diff_acc'  : 0, # Accumulated differences (dl - ts)
  'diff_min'  : 0, # Minimum difference
  'diff_max'  : 0, # Maximum difference
  }
