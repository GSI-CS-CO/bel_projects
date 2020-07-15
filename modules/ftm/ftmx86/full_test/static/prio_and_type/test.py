testcase = TestCase(
  "Cmd_Queue_Priorities_and_Types",
  [ Op("Init0_1", "dm-cmd", "halt"),
    Op("Init0_1", "dm-sched", "clear"),
    Op("Init0_2", "dm-sched", "add", "0", "test0_sched.dot"),
    Op("Test0_0", "dm-cmd", "-i", "0", "test0_cmd.dot"),
    Op("Test0_1_Rel_TValid", "dm-cmd", "rawqueue BLOCK_IN0", "0", None, "test0_0_exp.txt", "reltime"),
    Op("Test0_2_Rel_TValid", "dm-cmd", "rawqueue BLOCK_IN1", "0", None, "test0_1_exp.txt", "reltime"),
    Op("Init1_0", "dm-sched", "clear"),
    Op("Init1_1", "dm-sched", "add", "0", "test0_sched.dot"),
    Op("Test1_0", "dm-cmd", "-i", "0", "test1_cmd.dot"),
    Op("Test1_1_Abs_TValid", "dm-cmd", "rawqueue BLOCK_IN0", "0", None, "test1_0_exp.txt"),
    Op("Test1_2_Abs_TValid", "dm-cmd", "rawqueue BLOCK_IN1", "0", None, "test1_1_exp.txt"),
  ]
)