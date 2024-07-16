import dm_testbench

"""Class tests the memory limit for single CPUs and the whole datamaster.
"""
class UnitTestMemory(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp()
    if self.cpuQuantity == 3:
      self.file1 = 'groups_3_nonDefaultPatterns_9_blocksPerPattern_150.dot'
      if self.threadQuantity == 8:
        self.file2 = 'groups_3_nonDefaultPatterns_9_blocksPerPattern_10b.dot'
      else:
        self.file2 = 'groups_3_nonDefaultPatterns_9_blocksPerPattern_10c.dot'
    else:
      self.file1 = 'groups_4_nonDefaultPatterns_9_blocksPerPattern_150.dot'
      if self.threadQuantity == 8:
        self.file2 = 'groups_4_nonDefaultPatterns_9_blocksPerPattern_10b.dot'
      else:
        self.file2 = 'groups_4_nonDefaultPatterns_9_blocksPerPattern_10c.dot'

  def test_memory_cpu0(self):
    """Test for CPU 0. Add the schedule, add a second schedule: OK. When
    adding a third schedule, return code 250 comes back. This is expected.
    The add opration is rolled back. """
    self.addSchedule(self.file1)
    self.addSchedule(self.file2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedulesFolder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu0.dot'), [250])

  def test_memory_cpu1(self):
    """Test for CPU 1. Add the schedule, add a second schedule: OK. When
    adding a third schedule, return code 250 comes back. This is expected.
    The add opration is rolled back. """
    self.addSchedule(self.file1)
    self.addSchedule(self.file2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedulesFolder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu1.dot'), [250])

  def test_memory_cpu2(self):
    """Test for CPU 2. Add the schedule, add a second schedule: OK. When
    adding a third schedule, return code 250 comes back. This is expected.
    The add opration is rolled back. """
    self.addSchedule(self.file1)
    self.addSchedule(self.file2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedulesFolder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu2.dot'), [250])

  def test_memory_cpu3(self):
    """Test for CPU 3. Add the schedule, add a second schedule: OK. When
    adding a third schedule, return code 250 comes back. This is expected.
    The add opration is rolled back. """
    self.addSchedule(self.file1)
    self.addSchedule(self.file2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.schedulesFolder + 'groups_1_nonDefaultPatterns_9_blocksPerPattern_10_cpu3.dot'), [250])

"""Class tests the memory limit for CPU 0 and for all 4 CPUs.

Structure of test:
Generate a schedule with n blocks. Add this to DM with dm-sched. Result
schould be 0 if schedule fits into memory and 250 otherwise. The limits
are defined in setUp method.
"""
class UnitTestMemoryFull(dm_testbench.DmTestbench):

  def setUp(self):
    super().setUp();
    self.lastCpu = self.cpuQuantity - 1
    if self.threadQuantity == 8:
      self.maxNodes = 1868
      self.maxNodesCpu3 = 1667
      if self.cpuQuantity == 3:
        self.maxNodesCpu3 = 1719
    else:
      self.maxNodes = 1839
      self.maxNodesCpu3 = 1634

  def test_memory_full_bad(self):
    """ Test the memory for one CPU with more nodes than allowed. The add-operation should stop with a rollback. """
    fileName = self.schedulesFolder + 'memory_full.dot'
    self.generate_schedule(fileName, self.maxNodes + 5)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_bad1(self):
    """ Test the memory for one CPU with more nodes than allowed. The add-operation should stop with a rollback. """
    fileName = self.schedulesFolder + 'memory_full.dot'
    self.generate_schedule(fileName, self.maxNodes + 1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_ok(self):
    """ Test the memory for one CPU with the maximum number of nodes."""
    fileName = self.schedulesFolder + 'memory_full_ok.dot'
    self.generate_schedule(fileName, self.maxNodes)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_overfull(self):
    """ Test the memory for one CPU with more nodes than allowed. The add-operation should stop with a rollback."""
    fileName = self.schedulesFolder + 'memory_overfull.dot'
    self.generate_schedule(fileName, self.maxNodes + 5)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=2)
    self.deleteFile(fileName)

  def test_memory_full_4cpuOK(self):
    """Test the memory for all 4 CPUs with the maximum number of nodes allowed."""
    fileName = self.schedulesFolder + 'memory_full_4cpuOK_cpu0.dot'
    self.generate_schedule(fileName, self.maxNodes, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedulesFolder + 'memory_full_4cpuOK_cpu1.dot'
    self.generate_schedule(fileName, self.maxNodes, 1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    if self.cpuQuantity == 4:
      fileName = self.schedulesFolder + 'memory_full_4cpuOK_cpu2.dot'
      self.generate_schedule(fileName, self.maxNodes, 2)
      self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
          fileName), [0], linesCout=0, linesCerr=0)
      self.deleteFile(fileName)
    fileName = self.schedulesFolder + 'memory_full_4cpuOK_cpu3.dot'
    self.generate_schedule(fileName, self.maxNodesCpu3, self.lastCpu)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_4cpuFail(self):
    """Test the memory for all 4 CPUs with exactly one node more than allowed."""
    fileName = self.schedulesFolder + 'memory_full_4cpuFail_cpu0.dot'
    self.generate_schedule(fileName, self.maxNodes, 0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    fileName = self.schedulesFolder + 'memory_full_4cpuFail_cpu1.dot'
    self.generate_schedule(fileName, self.maxNodes, 1)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    if self.cpuQuantity == 4:
      fileName = self.schedulesFolder + 'memory_full_4cpuFail_cpu2.dot'
      self.generate_schedule(fileName, self.maxNodes, 2)
      self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
          fileName), [0], linesCout=0, linesCerr=0)
      self.deleteFile(fileName)
    fileName = self.schedulesFolder + 'memory_full_4cpuFail_cpu3.dot'
    self.generate_schedule(fileName, self.maxNodesCpu3 + 1, self.lastCpu)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)
    self.deleteFile(fileName)

  def generate_schedule(self, fileName, numberOfBlocks, cpu=0):
    """ Generates a schedule with numberOfBlocks nodes of type block.
    This schedule is not intended for running.

    :param fileName: the file to store the schedule.
    :param numberOfBlocks: number of nodes.
    :param cpu:  CPU for the schedule (Default value = 0)

    """
    lines = []
    lines.append('digraph memFull {')
    lines.append(f'node [cpu={cpu} type=block tperiod=1000]')
    for i in range(numberOfBlocks):
      lines.append(f'Block{cpu}_{i:04d} [pattern=Pattern{cpu}_{i:04d} patentry=1 patexit=1]')
    lines.append('}')
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))

  def test_memory_full_msg_small(self):
    """Test generates a schedule with one block and 10 timing messages in one loop.
    This test is a preparation for the larger test and checks the generated schedule file.
    """
    fileName = self.schedulesFolder + 'memory_full_msg_small.dot'
    patternName = 'PatternMsgSmall'
    self.generate_schedule_msg(fileName, patternName, 10)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', patternName),
        [0], linesCout=1, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_msg_half(self):
    """ Test the memory with about half the number of allowed nodes."""
    fileName = self.schedulesFolder + 'memory_full_msg_half.dot'
    patternName = 'PatternMsgHalf'
    self.generate_schedule_msg(fileName, patternName, 900)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', patternName),
        [0], linesCout=1, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_msg_ok(self):
    """ Test the memory with the maximum number of allowed nodes.
    Add the schedule and start the pattern.
    """
    fileName = self.schedulesFolder + 'memory_full_msg.dot'
    patternName = 'PatternMsgOk'
    self.generate_schedule_msg(fileName, patternName, self.maxNodes - 2)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', patternName),
        [0], linesCout=1, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_msg_infinite_loop_ok(self):
    """ The schedule has 1000 tmsg nodes. Test that the limit of 1000 for
    the iteration count works.
    If the pattern name is longer than 24 chars we get exceptions depending
    on the length of the pattern name and the number of nodes.
    963 nodes: return code -11
    965 nodes: return code -6, "terminate called after throwing an instance of 'boost::archive::archive_exception'", '  what():  input stream error'
    970 nodes: stderr: ['*** stack smashing detected ***: terminated']
    """
    fileName = self.schedulesFolder + 'memory_full_msg_infinte_loop_ok.dot'
    patternName = 'PatMsgInfiniteLoopOK'
    self.generate_schedule_msg(fileName, patternName, 1000, split=False)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', patternName),
        [0], linesCout=1, linesCerr=0)
    self.deleteFile(fileName)

  def test_memory_full_msg_infinite_loop_fail(self):
    """ The schedule has 1001 tmsg nodes to test the error message
    dm-sched: Failed to execute <add. Cause: Validation of Event Sequence: Node 'Msg0_0000'
    of type 'tmsg' is probably part of an infinite loop (iteration cnt > 1000)
    """
    fileName = self.schedulesFolder + 'memory_full_msg_infinte_loop_Fail.dot'
    patternName = 'PatternMsgInfiniteLoopFail'
    self.generate_schedule_msg(fileName, patternName, 1001, split=False)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)
    self.deleteFile(fileName)

  def generate_schedule_msg(self, fileName, patternName, numberOfMsgs, cpu=0, split=True, offset=400000):
    """Generate a schedule and write it to a file. The schedule has one block and timing messages.
    The timing messages have a name counting from 0 to numberOfMsgs - 1. toffs is the number in
    the name multiplied by offset. If the number of messages is above 1000, the nodes are split into
    two loops if split is True. The pattern is the same for all nodes.

    :param fileName: the name of the schedule file
    :param patternName: the name of the pattern used for all nodes
    :param numberOfMsgs: the number of timing message nodes
    :param split: split timing messages into two loops if more than 1000 timing messages (Default value = True)
    :param offset: toffset of timing messages in nano seconds (Default value = 400000). This is the delay between two timing messages
    :param cpu: the CPU to use (Default value = 0)

    """
    lines = []
    lines.append('digraph memFullMsg {')
    lines.append(f'node [cpu={cpu} type=tmsg pattern={patternName} fid=1]')
    # create the nodes
    lines.append(f'Block{cpu}_0 [type=block patentry=1 patexit=1 tperiod=1000000000]')
    for i in range(numberOfMsgs):
      lines.append(f'Msg{cpu}_{i:04d} [par={i} evtno={i} toffs={offset*i:d}]')
    lines.append(f'Block{cpu}_0 -> Msg{cpu}_0000 [type=defdst]')
    # create the edges
    limit1 = numberOfMsgs
    if numberOfMsgs > 1000 and split:
      limit1 = 1000
    for i in range(1,limit1):
      lines.append(f'Msg{cpu}_{i-1:04d} -> Msg{cpu}_{i:04d} [type=defdst]')
    if numberOfMsgs > 1000 and split:
      lines.append(f'Msg{cpu}_{limit1-1:04d} -> Block{cpu}_0 [type=defdst]')
      lines.append(f'Block{cpu}_0 -> Msg{cpu}_{limit1:04d} [type=altdst]')
      for i in range(limit1+1, numberOfMsgs):
        lines.append(f'Msg{cpu}_{i-1:04d} -> Msg{cpu}_{i:04d} [type=defdst]')
    lines.append(f'Msg{cpu}_{numberOfMsgs-1:04d} -> Block{cpu}_0 [type=defdst]')
    lines.append('}')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))
