#Test for locking and asynchronous queue manipulation

testcase = TestCase(
  "Test loop with flow initialiser",
  [ Op("Init0_0",               "dm-cmd", "reset all"),
    Op("Init0_1",               "dm-sched", "add", "0", "test_sched.dot"),
    Op("Test0_0_Run",           "dm-cmd", "startpattern IN_A"),
    Op("Test0_1_CheckVis",      "dm-sched", "rawvisited", "0.5", None,  "test0_0_exp.txt"),
    Op("Test0_2_CheckMCnt",     "dm-cmd", "rawstatus", "0.0", None, "test0_1_exp.txt"),
    Op("Test1_2_Run",           "dm-cmd", "startpattern IN_B"),
    Op("Test1_1_CheckVis",      "dm-sched", "rawvisited", "0.5", None,  "test1_0_exp.txt"),
    Op("Test1_2_CheckMCnt",     "dm-cmd", "rawstatus", "0.0", None, "test1_1_exp.txt"),
  ]
)