#Test for write lock behaviour, read lock behaviour and locking from cmd dot

testcase = TestCase(
  "Test Async Flush and Rewrite",
  [ Op("Init0_0", "cmd", "reset all"),
    Op("Init0_1", "sched", "add", "0", "test_sched.dot"),
    Op("Test0_2_Run", "cmd", "startpattern LOOP"),
    Op("Test0_3_CheckVis", "sched", "rawvisited", "0.0", None,  "test0_3_exp.txt"),
    Op("Test1_0_lock", "cmd", "-i", "0",                        "test1_0a_cmd.dot"),
    Op("Test1_1_CheckLock", "cmd", "showlocks", "0", None,      "test1_1_exp.txt"),
    Op("Test1_2_async_clear", "cmd", "-i", "0",                 "test1_0b_cmd.dot"),
    Op("Test1_3_Queue", "cmd", "rawqueue BLOCK_B", "0", None, "test1_3_exp.txt", "cmdqty",),
    Op("Test1_4_flow", "cmd", "-i", "0",                        "test1_0c_cmd.dot"),
    Op("Test1_5_Queue", "cmd", "rawqueue BLOCK_B", "0", None, "test1_5_exp.txt", ["reltime", "cmdqty"],),
    Op("Test1_6_unlock", "cmd", "-i", "1.0",                    "test1_0d_cmd.dot"),
    Op("Test1_7_CheckVis", "sched", "rawvisited", "2.0", None,  "test1_7_exp.txt"),
    Op("Test1_8_CheckLock", "cmd", "showlocks", "0", None,      "test1_8_exp.txt"),
    Op("Test1_9_Queue", "cmd", "rawqueue BLOCK_B", "0", None, "test1_9_exp.txt", ["reltime", "cmdqty"],)
  ]
)