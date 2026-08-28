import dm_testbench
import pytest

"""
Test cases for starting many threads with one startthread node.
"""
class SimultaneousThreads(dm_testbench.DmTestbench):

  def testSimultaneousThreadsAll(self):
    self.runSimultaneousThreads(self.threadQuantity)

  def testSimultaneousThreads4(self):
    self.runSimultaneousThreads(4)

  def testSimultaneousThreads2(self):
    self.runSimultaneousThreads(2)

  def runSimultaneousThreads(self, threads):
    """Use a schedule with a node of type startthread to start a number of
    threads at the same time (simultaneously).
    The startthread node is in a loop which produces messages with 50 Hz.
    """
    self.scheduleFile0 = f'simultaneousThreads{threads}.dot'
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    snoopFile = 'snoop_' + self.scheduleFile0.replace('.dot', '.csv')
    self.generateSchedule(self.schedulesFolder + self.scheduleFile0, threads)
    self.snoopToCsvWithAction(snoopFile, self.actionSimultaneousThreads, duration=3)
    counts = self.analyseDmCmdOutput('00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity], useVerbose=True)
    key1 = ''
    for key in counts:
      value = counts[key]
      if key1 == '' and int(value) > 0:
        key1 = key
        value1 = value
        self.assertEqual(value1, value, f'{key1=}: {value1} compared to {key=}: {value}')
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    keyList = {'0x0000000000000000': '>50', }
    for i in range(1, threads):
      keyList[f'0x{i:016x}'] = '>50'
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues=keyList, addDelayed=True)
    self.deleteFile(snoopFile)
    self.deleteFile(self.schedulesFolder + self.scheduleFile0)

  def actionSimultaneousThreads(self):
    """During snoop start pattern MAIN. Run it for 1.5 seconds and stop it.
    Download the schedule for later compare.
    """
    self.startPattern(self.scheduleFile0, 'MAIN')
    self.delay(1.5)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'MAIN'), [0], 0, 0)

  def generateSchedule(self, fileName, threadQuantity):
    """Generate a schedule and write it to a file.

    :param fileName: the name of the schedule file.
    :param threadQuantity: the number of threads to start simultaneously.
    """
    self.assertGreaterEqual(self.threadQuantity, threadQuantity, f'Number of threads ({threadQuantity}) should be at most {self.threadQuantity}.')
    self.assertGreaterEqual(threadQuantity, 2, f'Number of threads ({threadQuantity}) should be at least 2.')
    if threadQuantity == 32:
      threadMask = '0xfffffffe'
    elif threadQuantity == 8:
      threadMask = '0xfe'
    elif threadQuantity == 4:
      threadMask = '0xe'
    elif threadQuantity == 2:
      threadMask = '0x2'
    else:
      self.assertTrue(False, f'threadQuantity {threadQuantity} not supported.')
    lines = []
    lines.append(f'digraph "SimultaneousThreads{threadQuantity}" {{')
    lines.append(f'  name="SimultaneousThreads{threadQuantity}"')
    # create the MAIN loop
    lines.append(f'  node [cpu=1 toffs=0 pattern=MAIN fillcolor=white style=filled]')
    lines.append(f'  B_VARI [type=block shape=rectangle tperiod=200000]')
    lines.append(f'  StartThread [type=startthread shape=triangle color=cyan pattern=MAIN startoffs=0 thread="{threadMask}"]')
    lines.append(f'  Evt_MAIN [type=tmsg shape=oval fid=1 evtno=1 par=0 id="0x1000001000000000"]')
    lines.append(f'  B_MAIN [type=block shape=rectangle patexit=1 color=purple tperiod=20000000 qlo=1]')
    lines.append(f'  B_VARI -> StartThread -> Evt_MAIN -> B_MAIN -> B_VARI [type=defdst color=red]')
    # create the origin nodes
    lines.append(f'  node [type=origin shape=octagon]')
    for i in range(1,threadQuantity):
      if i == 1:
        attributes = ' patentry=1 color=darkorange3'
      else:
        attribute = ''
      lines.append(f'  Ori_T{i:02d} [thread={i:d}{attributes}]')
    # create the tmsg nodes
    lines.append(f'  node [type=tmsg shape=oval patentry=1 color=darkorange3 fid=1 evtno=2]')
    for i in range(1,threadQuantity):
      lines.append(f'  Evt_{i:02d} [pattern={i:02d} toffs={i * 300000} par={i:d} id="0x1000002000000000"]')
    # create the block nodes
    lines.append(f'  node [type=block shape=rectangle patentry=0 patexit=1 color=purple tperiod=10000000]')
    for i in range(1,threadQuantity):
      lines.append(f'  B_{i:02d} [pattern={i:02d}]')
    # create edges
    lines.append(f'  edge [type=defdst color=red]')
    for i in range(1,threadQuantity - 1):
      lines.append(f'  Ori_T{i:02d} -> Ori_T{i+1:02d}')
    lines.append(f'  Ori_T{threadQuantity - 1:02d} -> B_VARI')
    for i in range(1,threadQuantity):
      lines.append(f'  Evt_{i:02d} -> B_{i:02d}')
    lines.append(f'  edge [type=origindst color=gray]')
    for i in range(1,threadQuantity):
      lines.append(f'  Ori_T{i:02d} -> Evt_{i:02d}')
    # closing brace
    lines.append('}')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))
