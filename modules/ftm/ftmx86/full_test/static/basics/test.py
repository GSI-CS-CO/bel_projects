testcase = TestCase(
  "Test0_Basic_Schedule_Operations",
  [ Op("Test0_0", "sched", "clear -f"),
    Op("Test0_1_empty_schedule", "sched", "status", "0", None, "test0_0_exp.txt"),
    Op("Test1_0_add_simple_schedule", "sched", "add", "0", "test0_sched.dot"),
    Op("Test1_1_simple_schedule", "sched", "status", "0", None, "test1_0_exp.txt"),
    Op("Test2_0_rem_simple_schedule", "sched", "remove", "0", "test0_sched.dot"),
    Op("Test2_1_empty_schedule", "sched", "status", "0", None, "test0_0_exp.txt"),
  ]
)