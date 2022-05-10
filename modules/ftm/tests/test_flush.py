import dm_testbench
import pytest

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

  match = 'Qty: '
  lengthQ = len(match)

  """Get the quantity of command executions from the output line of 'dm-cmd <datamaster> queue -v <block name>'.
  Search for 'Qty: ' in the line and parse the number.
  """
  def getQuantity(self, line):
    pos = line.find(self.match)
    pos1 = line.find(' ', pos + self.lengthQ)
    quantity = 0
    if pos > -1 and pos1 > pos:
      quantity = int(line[pos + self.lengthQ:pos1])
    return quantity

  """ Check that a flush command is executed and the defined queues are flushed.
  queuesToFlush: binary value between 0 and 7 for the queues to flush.
  blockName: name of the block with the queues to check.
  flushPrio: priority of the flush command. Defines the queue where to look for the flush command.
  Priority queues higher than the flushPrio are not affected by the flush command since these
  queues are empty when a flush command with lower priority is executed. Therefore queuesToFlush is changed for
  flushprio = 0, 1.
  The check for the executed flush command and the checks for the flushed queues are independant. All checks
  are done in one parse run of the output of 'dm-cmd <datamaster> -v queue <blockName>'. Flags and counters
  are used to signal the section inside the output.
  """
  def check_queue_flushed(self, queuesToFlush, blockName, flushPrio):
    output = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, '-v', 'queue', blockName), [0], 38)
    checkQueue0 = False
    check0 = False
    checkQueue1 = False
    check1 = False
    checkQueue2 = False
    check2 = False
    checkFlush0 = False
    checkFlush1 = False
    checkFlush2 = False
    flushExecuted = False
    for line in output:
      # check that flush is executed (there is exactly 1 flush executed)
      if checkFlush0:
        # ~ print(f'0 {counterFlush0} {flushExecuted} flushPrio {flushPrio}, {"empty" in line}, {"CmdType: flush" in line}, qty: {self.getQuantity(line)}')
        flushExecuted = flushExecuted or (flushPrio == 0 and 'empty' in line and 'CmdType: flush' in line and self.getQuantity(line) == 0)
        counterFlush0 = counterFlush0 + 1
        if counterFlush0 > 3:
          checkFlush0 = False
      if flushPrio == 0 and 'Priority 0 (' in line:
        checkFlush0 = True
        counterFlush0 = 0
      if checkFlush1:
        # ~ print(f'1 {counterFlush1} {flushExecuted} flushPrio {flushPrio}, {"empty" in line}, {"CmdType: flush" in line}, qty: {self.getQuantity(line)}')
        flushExecuted = flushExecuted or (flushPrio == 1 and 'empty' in line and 'CmdType: flush' in line and self.getQuantity(line) == 0)
        counterFlush1 = counterFlush1 + 1
        if counterFlush1 > 3:
          checkFlush1 = False
      if flushPrio == 1 and 'Priority 1 (' in line:
        checkFlush1 = True
        counterFlush1 = 0
      if checkFlush2:
        # ~ print(f'2 {counterFlush2} {flushExecuted} flushPrio {flushPrio}, {"empty" in line}, {"CmdType: flush" in line}, qty: {self.getQuantity(line)}')
        flushExecuted = flushExecuted or (flushPrio == 2 and 'empty' in line and 'CmdType: flush' in line and self.getQuantity(line) == 0)
        counterFlush2 = counterFlush2 + 1
        if counterFlush2 > 3:
          checkFlush2 = False
      if flushPrio == 2 and 'Priority 2 (' in line:
        checkFlush2 = True
        counterFlush2 = 0

      # check that queues are flushed (0 to 3 queues can be flushed)
      if checkQueue0:
        check0 = ('empty' in line and self.getQuantity(line) > 0)
        counter0 = counter0 + 1
        if counter0 > 3:
          checkQueue0 = False
      if not checkQueue0 and queuesToFlush & 0x1 and 'Priority 0 (' in line:
        # start check of queue 0
        checkQueue0 = True
        counter0 = 0
      if checkQueue1:
        check1 = ('empty' in line and self.getQuantity(line) > 0)
        counter1 = counter1 + 1
        if counter1 > 3:
          checkQueue1 = False
      if not checkQueue1 and queuesToFlush & 0x2 and 'Priority 1 (' in line:
        # start check of queue 1
        checkQueue1 = True
        counter1 = 0
      if checkQueue2:
        check2 = ('empty' in line and self.getQuantity(line) > 0)
        counter2 = counter2 + 1
        if counter2 > 3:
          checkQueue2 = False
      if not checkQueue2 and queuesToFlush & 0x4 and 'Priority 2 (' in line:
        # start check of queue 2
        checkQueue2 = True
        counter2 = 0
    if flushPrio == 0 and queuesToFlush > 1:
      queuesToFlush = queuesToFlush & 0x1
    if flushPrio == 1 and queuesToFlush > 1:
      queuesToFlush = queuesToFlush & 0x3
    queuesFlushed = 0
    if queuesToFlush & 0x1 and check0:
      queuesFlushed = queuesFlushed + 1
    if queuesToFlush & 0x2 and check1:
      queuesFlushed = queuesFlushed + 2
    if queuesToFlush & 0x4 and check2:
      queuesFlushed = queuesFlushed + 4
    self.assertEqual(queuesFlushed, queuesToFlush, f'Queue 2 flushed {check2}, Queue 1 flushed {check1}, Queue 0 flushed {check0}, Queues to check: {queuesToFlush:03b}')
    self.assertTrue(flushExecuted, f'flushExecuted: {flushExecuted}, queuesToFlush: {queuesToFlush}')

  def test_flow_flushNone_prio0(self):
    self.addSchedule('flush-queue01-prio0.dot')
    # flush no queue with prio 0
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x0, 'Block0E', 0)

  def test_flow_flush0_prio0(self):
    self.addSchedule('flush-queue01-prio0.dot')
    # flush queue 0 (low) with prio 0
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x1, 'Block1E', 0)

  def test_flow_flush1_prio0(self):
    self.addSchedule('flush-queue01-prio0.dot')
    # flush queue 1 (high) with prio 0 - does nothing, queue 1 is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x2, 'Block2E', 0)

  def test_flow_flush2_prio0(self):
    self.addSchedule('flush-queue01-prio0.dot')
    # flush queue 2 (interlock) with prio 0 - does nothing, queue 2 is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x4, 'Block3E', 0)


  def test_flow_flushNone_prio1(self):
    self.addSchedule('flush-queue01-prio1.dot')
    # flush no queue with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio1'), [0])
    self.delay(0.2)
    self.check_queue_flushed(0x0, 'Block0E', 1)

  def test_flow_flush0_prio1(self):
    self.addSchedule('flush-queue01-prio1.dot')
    # flush queue 0 (low) with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio1'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x1, 'Block1E', 1)

  def test_flow_flush1_prio1(self):
    self.addSchedule('flush-queue01-prio1.dot')
    # flush queue 1 (high) with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio1'), [0])
    self.delay(0.2)
    self.check_queue_flushed(0x2, 'Block2E', 1)

  def test_flow_flush2_prio1(self):
    self.addSchedule('flush-queue01-prio1.dot')
    # flush queue 2 (interlock) with prio 1 - does nothing, queue 2 is empty when queue 1 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio1'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x4, 'Block3E', 1)


  def test_flow_flushNone_prio2(self):
    self.addSchedule('flush-queue01-prio2.dot')
    # flush no queue with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queueNone-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x0, 'Block0E', 2)

  def test_flow_flush0_prio2(self):
    self.addSchedule('flush-queue01-prio2.dot')
    # flush queue 0 (low) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue0-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x1, 'Block1E', 2)

  def test_flow_flush1_prio2(self):
    self.addSchedule('flush-queue01-prio2.dot')
    # flush queue 1 (high) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue1-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x2, 'Block2E', 2)

  def test_flow_flush2_prio2(self):
    self.addSchedule('flush-queue01-prio2.dot')
    # flush queue 2 (interlock) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue2-prio2'), [0])
    self.delay(0.2)
    self.check_queue_flushed(0x4, 'Block3E', 2)


  def test_flow_flush01_prio0(self):
    self.addSchedule('flush-queue23-prio0.dot')
    # flush queues 0 (low), 1 (high) with prio 0 - no effect for queue 1, it is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x3, 'Block0E', 0)

  def test_flow_flush02_prio0(self):
    self.addSchedule('flush-queue23-prio0.dot')
    # flush queues 0 (low), 2 (interlock) with prio 0 - no effect for queue 2, it is empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x5, 'Block1E', 0)

  def test_flow_flush12_prio0(self):
    self.addSchedule('flush-queue23-prio0.dot')
    # flush queues 1 (high), 2 (interlock) with prio 0 - does nothing, queues 1, 2 are empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x6, 'Block2E', 0)

  def test_flow_flush123_prio0(self):
    self.addSchedule('flush-queue23-prio0.dot')
    # flush queues 0 (low), 1 (high), 2 (interlock) with prio 0 - no effect for queues 1,2, queues 1,2 are empty when queue 0 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio0'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio0'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x7, 'Block3E', 0)


  def test_flow_flush01_prio1(self):
    self.addSchedule('flush-queue23-prio1.dot')
    # flush queues 0 (low), 1 (high) with prio 1
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio1'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x3, 'Block0E', 1)

  def test_flow_flush02_prio1(self):
    self.addSchedule('flush-queue23-prio1.dot')
    # flush queues 0 (low), 2 (interlock) with prio 1 - no effect for queue 2, it is empty when queue 1 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio1'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x5, 'Block1E', 1)

  def test_flow_flush12_prio1(self):
    self.addSchedule('flush-queue23-prio1.dot')
    # flush queues 1 (high), 2 (interlock) with prio 1 - no effect for queue 2, it is empty when queue 1 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio1'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x6, 'Block2E', 1)

  def test_flow_flush123_prio1(self):
    self.addSchedule('flush-queue23-prio1.dot')
    # flush queues 0 (low), 1 (high), 2 (interlock) with prio 0 - no effect for queue 2, queue 2 is empty when queue 1 is executed.
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio1'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio1'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x7, 'Block3E', 1)


  def test_flow_flush01_prio2(self):
    self.addSchedule('flush-queue23-prio2.dot')
    # flush queues 0 (low), 1 (high) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue01-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x3, 'Block0E', 2)

  def test_flow_flush02_prio2(self):
    self.addSchedule('flush-queue23-prio2.dot')
    # flush queues 0 (low), 2 (interlock) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue02-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x5, 'Block1E', 2)

  def test_flow_flush12_prio2(self):
    self.addSchedule('flush-queue23-prio2.dot')
    # flush queues 1 (high), 2 (interlock) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue12-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x6, 'Block2E', 2)

  def test_flow_flush123_prio2(self):
    self.addSchedule('flush-queue23-prio2.dot')
    # flush queues 0 (low), 1 (high), 2 (interlock) with prio 2
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio2'), [0])
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'P-queue123-prio2'), [0])
    self.delay(0.1)
    self.check_queue_flushed(0x7, 'Block3E', 2)
