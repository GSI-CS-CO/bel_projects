import dm_testbench

"""Class tests various variants of the flush command.

Naming of the tests:
test_flow_flushX_prioY, where
X are the queues to flush. Allowed values: None, 0, 1, 2, 01, 02, 12, 123.
Y is the priority of the flush. Allowed values: 0, 1, 2.
Naming of patterns:
P-queueX-prioY, where X and Y as described above.

Patterns used:
schedules/flush-queue01-prio0.dot
schedules/flush-queue01-prio1.dot
schedules/flush-queue01-prio2.dot
schedules/flush-queue23-prio0.dot
schedules/flush-queue23-prio1.dot
schedules/flush-queue23-prio2.dot

Structure of each test:
add the schedule, start the pattern twice. The second startpattern executes the commands which are written
to the queues on first startpattern.
Check for flushed queues and the executed flush command after a delay of 0.1 seconds.
Four tests use the same schedule to minimize the numer of schedules. Each schedule uses 4 CPUs.
"""
class UnitTestFlush(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    if self.cpuQuantity == 3:
      self.scheduleFile01Prio0 = 'flush-queue01-prio0-3cpu.dot'
      self.scheduleFile01Prio1 = 'flush-queue01-prio1-3cpu.dot'
      self.scheduleFile01Prio2 = 'flush-queue01-prio2-3cpu.dot'
      self.scheduleFile23Prio0 = 'flush-queue23-prio0-3cpu.dot'
      self.scheduleFile23Prio1 = 'flush-queue23-prio1-3cpu.dot'
      self.scheduleFile23Prio2 = 'flush-queue23-prio2-3cpu.dot'
    else:
      self.scheduleFile01Prio0 = 'flush-queue01-prio0.dot'
      self.scheduleFile01Prio1 = 'flush-queue01-prio1.dot'
      self.scheduleFile01Prio2 = 'flush-queue01-prio2.dot'
      self.scheduleFile23Prio0 = 'flush-queue23-prio0.dot'
      self.scheduleFile23Prio1 = 'flush-queue23-prio1.dot'
      self.scheduleFile23Prio2 = 'flush-queue23-prio2.dot'

  def test_flow_flushNone_prio0(self):
    self.addSchedule(self.scheduleFile01Prio0)
    # flush no queue with prio 0
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio0'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x0, 'Block0E', 0)

  def test_flow_flush0_prio0(self):
    self.addSchedule(self.scheduleFile01Prio0)
    # flush queue 0 (low) with prio 0
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio0'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x1, 'Block1E', 0)

  def test_flow_flush1_prio0(self):
    self.addSchedule(self.scheduleFile01Prio0)
    # flush queue 1 (high) with prio 0 - does nothing, queue 1 is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio0'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x2, 'Block2E', 0)

  def test_flow_flush2_prio0(self):
    self.addSchedule(self.scheduleFile01Prio0)
    if self.cpuQuantity > 3:
      # flush queue 2 (interlock) with prio 0 - does nothing, queue 2 is empty when queue 0 is executed.
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio0'), [0])
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio0'), [0])
      self.delay(0.1)
      self.checkQueueFlushed(0x4, 'Block3E', 0)


  def test_flow_flushNone_prio1(self):
    self.addSchedule(self.scheduleFile01Prio1)
    # flush no queue with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio1'), [0])
    self.delay(0.2)
    self.checkQueueFlushed(0x0, 'Block0E', 1)

  def test_flow_flush0_prio1(self):
    self.addSchedule(self.scheduleFile01Prio1)
    # flush queue 0 (low) with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio1'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x1, 'Block1E', 1)

  def test_flow_flush1_prio1(self):
    self.addSchedule(self.scheduleFile01Prio1)
    # flush queue 1 (high) with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio1'), [0])
    self.delay(0.2)
    self.checkQueueFlushed(0x2, 'Block2E', 1)

  def test_flow_flush2_prio1(self):
    self.addSchedule(self.scheduleFile01Prio1)
    if self.cpuQuantity > 3:
      # flush queue 2 (interlock) with prio 1 - does nothing, queue 2 is empty when queue 1 is executed.
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio1'), [0])
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio1'), [0])
      self.delay(0.1)
      self.checkQueueFlushed(0x4, 'Block3E', 1)


  def test_flow_flushNone_prio2(self):
    self.addSchedule(self.scheduleFile01Prio2)
    # flush no queue with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio2'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x0, 'Block0E', 2)

  def test_flow_flush0_prio2(self):
    self.addSchedule(self.scheduleFile01Prio2)
    # flush queue 0 (low) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio2'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x1, 'Block1E', 2)

  def test_flow_flush1_prio2(self):
    self.addSchedule(self.scheduleFile01Prio2)
    # flush queue 1 (high) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio2'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x2, 'Block2E', 2)

  def test_flow_flush2_prio2(self):
    self.addSchedule(self.scheduleFile01Prio2)
    if self.cpuQuantity > 3:
      # flush queue 2 (interlock) with prio 2
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio2'), [0])
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio2'), [0])
      self.delay(0.2)
      self.checkQueueFlushed(0x4, 'Block3E', 2)


  def test_flow_flush01_prio0(self):
    self.addSchedule(self.scheduleFile23Prio0)
    # flush queues 0 (low), 1 (high) with prio 0 - no effect for queue 1, it is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio0'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x3, 'Block0E', 0)

  def test_flow_flush02_prio0(self):
    self.addSchedule(self.scheduleFile23Prio0)
    # flush queues 0 (low), 2 (interlock) with prio 0 - no effect for queue 2, it is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio0'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x5, 'Block1E', 0)

  def test_flow_flush12_prio0(self):
    self.addSchedule(self.scheduleFile23Prio0)
    # flush queues 1 (high), 2 (interlock) with prio 0 - does nothing, queues 1, 2 are empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio0'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x6, 'Block2E', 0)

  def test_flow_flush123_prio0(self):
    self.addSchedule(self.scheduleFile23Prio0)
    if self.cpuQuantity > 3:
      # flush queues 0 (low), 1 (high), 2 (interlock) with prio 0 - no effect for queues 1,2, queues 1,2 are empty when queue 0 is executed.
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio0'), [0])
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio0'), [0])
      self.delay(0.1)
      self.checkQueueFlushed(0x7, 'Block3E', 0)


  def test_flow_flush01_prio1(self):
    self.addSchedule(self.scheduleFile23Prio1)
    # flush queues 0 (low), 1 (high) with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio1'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x3, 'Block0E', 1)

  def test_flow_flush02_prio1(self):
    self.addSchedule(self.scheduleFile23Prio1)
    # flush queues 0 (low), 2 (interlock) with prio 1 - no effect for queue 2, it is empty when queue 1 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio1'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x5, 'Block1E', 1)

  def test_flow_flush12_prio1(self):
    self.addSchedule(self.scheduleFile23Prio1)
    # flush queues 1 (high), 2 (interlock) with prio 1 - no effect for queue 2, it is empty when queue 1 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio1'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x6, 'Block2E', 1)

  def test_flow_flush123_prio1(self):
    self.addSchedule(self.scheduleFile23Prio1)
    if self.cpuQuantity > 3:
      # flush queues 0 (low), 1 (high), 2 (interlock) with prio 0 - no effect for queue 2, queue 2 is empty when queue 1 is executed.
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio1'), [0])
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio1'), [0])
      self.delay(0.1)
      self.checkQueueFlushed(0x7, 'Block3E', 1)


  def test_flow_flush01_prio2(self):
    self.addSchedule(self.scheduleFile23Prio2)
    # flush queues 0 (low), 1 (high) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio2'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x3, 'Block0E', 2)

  def test_flow_flush02_prio2(self):
    self.addSchedule(self.scheduleFile23Prio2)
    # flush queues 0 (low), 2 (interlock) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio2'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x5, 'Block1E', 2)

  def test_flow_flush12_prio2(self):
    self.addSchedule(self.scheduleFile23Prio2)
    # flush queues 1 (high), 2 (interlock) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio2'), [0])
    self.delay(0.1)
    self.checkQueueFlushed(0x6, 'Block2E', 2)

  def test_flow_flush123_prio2(self):
    self.addSchedule(self.scheduleFile23Prio2)
    if self.cpuQuantity > 3:
      # flush queues 0 (low), 1 (high), 2 (interlock) with prio 2
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio2'), [0])
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio2'), [0])
      self.delay(0.1)
      self.checkQueueFlushed(0x7, 'Block3E', 2)
