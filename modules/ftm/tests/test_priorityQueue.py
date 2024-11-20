import dm_testbench
import datetime

"""Class TestPriorityQueue tests the execution of priority queues.
"""
class TestPriorityQueue(dm_testbench.DmTestbench):

  def generateScheduleAltdestinations(self, fileName, numDestinations, patternName, period):
    """Generate a schedule with numDestinations altdst edges to a central block.
    These edges connect this central block with tmsg nodes. The tmsg nodes
    have a defdst edge to the central block.

    :param fileName: the name of the schedule file
    :param patternName: the name of the pattern used for all nodes
    :param numDestinations: the number of timing message nodes, maximal maxNumberDestinations.
    :param period: the time period for the central block
    :param cpu: the CPU to use (Default value = 0)

    """
    maxNumberDestinations = 1000
    cpu = 0
    blockName = f'Block{cpu}'
    self.assertGreater(maxNumberDestinations, numDestinations, f'Number of messages ({numDestinations}) should be less than {maxNumberDestinations}.')
    self.assertGreater(numDestinations, 1, f'Number of messages ({numDestinations}) should be greater than 1 (Pattern {patternName}).')
    offset = int(period / numDestinations)
    offsetSec = offset / 1000000000
    # ~ print(f'Generate: {numDestinations=:d}, {period=:d}, {offsetSec=}')
    lines = []
    lines.append(f'digraph AltDestLists{numDestinations}' + ' {')
    lines.append(f'node [cpu={cpu} type=tmsg pattern={patternName} fid=1 toffs=0]')
    lines.append(f'edge [type=defdst]')
    # create the nodes
    lines.append(f'{blockName} [type=block patentry=1 patexit=1 qlo=1 tperiod={period:d}]')
    for i in range(numDestinations):
      lines.append(f'Msg{cpu}_{i:04d} [par={i} gid={numDestinations} evtno={i}]')
    # create the edges
    lines.append(f'{blockName} -> Msg{cpu}_0000')
    lines.append(f'Msg{cpu}_0000 -> {blockName}')
    for i in range(1,numDestinations):
      lines.append(f'Msg{cpu}_{i:04d} -> {blockName}')
      lines.append(f'{blockName} -> Msg{cpu}_{i:04d} [type=altdst]')
    lines.append('}')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))

  def switchAction(self, argsList):
    numDestinations = argsList[0]
    frequency = argsList[1]
    expectedReturnCode = 0
    if len(argsList) > 2:
      expectedReturnCode = argsList[2]
    delay = 0.05
    # ~ print(f'Action: {numDestinations=}, {frequency=}, {delay=}')
    for i in range(1, numDestinations):
      command = (self.binaryDmCmd, self.datamaster, 'flow', 'Block0', f'Msg0_{i:04d}')
      try:
        # ~ print('Action', i, command, datetime.datetime.now())
        # ~ print ('\n'.join(self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'queue', 'Block0'), [0], 10, 0)))
        self.startAndCheckSubprocess(command, [expectedReturnCode], 0, 0)
      except AssertionError as aError:
        self.delay(delay)
        if 'wrong return code -6' in aError.args[0]:
          # ~ print('Action retry', i, command, datetime.datetime.now())
          # ~ print ('\n'.join(self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'queue', 'Block0'), [0], 10, 0)))
          self.startAndCheckSubprocess(command, [0], 0, 0)
      finally:
        self.delay(delay)

  def runFlow(self, numDestinations, expectError=False):
    """With a generated schedule test altdst. Use a loop over all tmsg
    nodes to switch the destinations such that the schedule flow from the
    central block switches through all tmsg nodes.
    """
    scheduleFile = f'priorityQueue-{numDestinations}.dot'
    fileCsv = f'priorityQueue-{numDestinations}.csv'
    patternName = f'PriorityQueue{numDestinations:04d}'
    downloadFile = scheduleFile.replace('.dot', '-download.dot')
    # time period for 1Hz
    frequency = 1
    period = int(1000 * 1000 * 1000 / 2 / frequency)
    self.generateScheduleAltdestinations(self.schedulesFolder + scheduleFile, numDestinations, patternName, period)
    # add schedule and start pattern, snoop for some time
    self.startPattern(scheduleFile, patternName)
    # large numDestinations require more time for snoop. The last part handles this.
    snoopTime = 1 + max(2, int(numDestinations / 2 / frequency)) + int(numDestinations / 50)
    print(f'Run: {numDestinations=}, {frequency=}, {period=}, {snoopTime=}')
    if expectError:
      returnCode = -6
    else:
      returnCode = 0
    self.snoopToCsvWithAction(fileCsv, self.switchAction, actionArgs=[numDestinations, frequency, returnCode], duration=snoopTime)
    # check downloaded schedule
    options = '-so'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', options, downloadFile), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', self.schedulesFolder + downloadFile, downloadFile), [0], 0, 0)
    self.deleteFile(downloadFile)
    # analyze snoop file (csv)
    keyList = {'0x0000000000000000': '>0', }
    for i in range(1, min(5, numDestinations)):
      keyList[f'0x{i:016x}'] = '>0'
    self.analyseFrequencyFromCsv(fileCsv, 20, checkValues=keyList)
    # cleanup
    self.deleteFile(self.schedulesFolder + scheduleFile)
    self.deleteFile(fileCsv)

  def test_flow0005(self):
    self.runFlow(5)

  def test_flow0006(self):
    self.runFlow(6, expectError=True)
