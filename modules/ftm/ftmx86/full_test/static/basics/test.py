testcase = TestCase(
  "Basic_Schedule_Operations",
  [ Op("Test0_0", "dm-cmd", "halt"),
    Op("Test0_0", "dm-sched", "clear -f"),
    Op("Test0_1_empty_schedule", "dm-sched", "status", "0", None, "test0_0_exp.txt"),
    Op("Test1_0_add_simple_schedule", "dm-sched", "add", "0", "test0_sched.dot"),
    Op("Test1_1_simple_schedule", "dm-sched", "status", "0", None, "test1_0_exp.txt"),
    Op("Test2_0_rem_simple_schedule", "dm-sched", "remove", "0", "test0_sched.dot"),
    Op("Test2_1_empty_schedule", "dm-sched", "status", "0", None, "test0_0_exp.txt"),
  ]
)