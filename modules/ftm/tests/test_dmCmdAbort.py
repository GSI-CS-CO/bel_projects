import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'abort'.
Main focus is testing with bit masks for CPUs and threads.
"""
class AbortTests(dm_testbench.DmTestbench):

  def testAbortRunningThreads(self):
    """Prepare all threads on all CPUs.
    Abort some threads. Check that these are not running.
    """
    self.prepareRunThreads()
    # Abort some threads on CPUs 0 and 1
    cpu = '0x3' # CPUs 0 and 1
    thread = '0xaa' # Threads 1, 3, 5, 7
    threadCount = self.bitCount(thread, self.threadQuantity)
    cpuCount = self.bitCount(cpu, self.cpuQuantity)
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', f'{cpu}', '-t', f'{thread}', 'abort'), [0], threadCount * cpuCount, 0)
    # ~ self.printStdOutStdErr(lines)
    threads = self.listFromBits(thread, self.threadQuantity)
    cpus = self.listFromBits(cpu, self.cpuQuantity)
    allCpuMask = self.maskFromList(range(self.cpuQuantity), self.cpuQuantity)
    for i in range(cpuCount):
      for j in range(threadCount):
        self.assertEqual(lines[0][i*threadCount+j], f'CPU {cpus[i]} Thread {threads[j]} aborted.', 'wrong output')
    # Check that the remaining threads are running
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, '-c', allCpuMask, 'running'), [0], self.cpuQuantity, 0)
    # ~ self.printStdOutStdErr(lines)
    # define the thread masks for 32 and 8 threads.
    if self.threadQuantity == 32:
      threadMask = '0xffffffff'
      threadMaskAborted = '0xffffff55'
    elif self.threadQuantity == 8:
      threadMask = '0xff'
      threadMaskAborted = '0x55'
    else:
      self.assertFalse(True, f'threadQuantity is {self.threadQuantity}, allowed: 8 or 32')
    # compare the lines of stdout with expected texts.
    for i in range(self.cpuQuantity):
      if i in cpus:
        expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMaskAborted)
      else:
        expectedText = 'CPU {variable} Running Threads: {mask}'.format(variable=i, mask=threadMask)
      messageText = 'wrong output, expected: ' + expectedText
      self.assertEqual(lines[0][i], expectedText, messageText)

  def testAbortSingleThreadDecimal(self):
    """Loop over all CPUs and all threads aborting this thread.
    Uses the thread number in decimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, thread, 'abort')

  def testAbortSingleThreadHex(self):
    """Loop over all CPUs and all threads aborting this thread.
    Uses the thread number in hexadecimal form.
    """
    for cpu in range(self.cpuQuantity):
      for thread in range(self.threadQuantity):
        self.runThreadXCommand(cpu, f'0x{(1 << thread):x}', 'abort')

  def tearDown(self):
    super().tearDown()
    # reset all CPUs to get a clean state. This is not done by dm-cmd reset all.
    self.resetAllCpus()
