import dm_testbench

"""Class TestAltDestinationLists tests the limit of the extended lists
of altdst edges for a block.
"""
class TestAltDestinationLists(dm_testbench.DmTestbench):

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
    self.assertGreater(maxNumberDestinations, numDestinations, f'Number of messages ({numDestinations}) should be less than {maxNumberDestinations}.')
    self.assertGreater(numDestinations, 1, f'Number of messages ({numDestinations}) should be greater than 1 (Pattern {patternName}).')
    offset = int(period / numDestinations)
    offsetSec = offset / 1000000000
    print(f'Generate: {numDestinations=:d}, {period=:d}, {offsetSec=}')
    lines = []
    lines.append(f'digraph AltDestLists{numDestinations}' + ' {')
    lines.append(f'node [cpu={cpu} type=tmsg pattern={patternName} fid=1]')
    lines.append(f'edge [type=defdst]')
    # create the nodes
    lines.append(f'Block{cpu}_0 [type=block patentry=1 patexit=1 qlo=1 tperiod={period:d}]')
    for i in range(numDestinations):
      lines.append(f'Msg{cpu}_{i:04d} [par={i} gid={numDestinations} evtno={i} toffs={offset*i:d}]')
    # create the edges
    lines.append(f'Block{cpu}_0 -> Msg{cpu}_0000')
    lines.append(f'Msg{cpu}_0000 -> Block{cpu}_0')
    for i in range(1,numDestinations):
      lines.append(f'Msg{cpu}_{i:04d} -> Block{cpu}_0')
      lines.append(f'Block{cpu}_0 -> Msg{cpu}_{i:04d} [type=altdst]')
    lines.append('}')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))

  def switchAction(self, argsList):
    numDestinations = argsList[0]
    frequency = argsList[1]
    delay = 1 / frequency * 0.52
    self.delay(0.5)
    print(f'Action: {numDestinations=}, {frequency=}, {delay=}')
    for i in range(1, numDestinations):
      command = (self.binaryDmCmd, self.datamaster, 'flow', 'Block0_0', f'Msg0_{i:04d}')
      try:
        self.startAndCheckSubprocess(command, [0], 0, 0)
      except AssertionError as aError:
        self.delay(delay)
        if 'wrong return code' in aError.args[0]:
          self.startAndCheckSubprocess(command, [0], 0, 0)
      finally:
        self.delay(delay)

  def runAltDestinationsX(self, numDestinations):
    """With a generated schedule test altdst. Use a loop over all tmsg
    nodes to switch the destinations such that the schedule flow from the
    central block switches through all tmsg nodes.
    """
    scheduleFile = f'altDestinations-{numDestinations}.dot'
    fileCsv = f'altDestinations-{numDestinations}.csv'
    patternName = f'AltDest{numDestinations:04d}'
    downloadFile = scheduleFile.replace('.dot', '-download.dot')
    # time period for 10Hz
    frequency = 10
    period = int(1000 * 1000 * 1000 / frequency)
    self.generateScheduleAltdestinations(self.schedulesFolder + scheduleFile, numDestinations, patternName, period)
    # add schedule and start pattern, snoop for some time
    self.startPattern(scheduleFile, patternName)
    # large numDestinations require more time for snoop. The last part handles this.
    snoopTime = int((1 + max(2, int(numDestinations / frequency)) + int(numDestinations / 50)) * 1.5)
    print(f'Run: {numDestinations=}, {frequency=}, {period=}, {snoopTime=}')
    self.snoopToCsvWithAction(fileCsv, self.switchAction, actionArgs=[numDestinations, frequency], duration=snoopTime)
    # check downloaded schedule
    options = '-so'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', options, downloadFile), [0], 0, 0)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', self.schedulesFolder + downloadFile, downloadFile), [0], 0, 0)
    self.deleteFile(downloadFile)
    # analyze snoop file (csv)
    keyList = {'0x0000000000000000': '>0', }
    for i in range(1, numDestinations):
      keyList[f'0x{i:016x}'] = '>0'
    self.analyseFrequencyFromCsv(fileCsv, 20, checkValues=keyList)
    # cleanup
    self.deleteFile(self.schedulesFolder + scheduleFile)
    self.deleteFile(fileCsv)

  def test_altDestinations0002(self):
    self.runAltDestinationsX(2)

  def test_altDestinations0005(self):
    self.runAltDestinationsX(5)

  def test_altDestinations0010(self):
    self.runAltDestinationsX(10)

  def test_altDestinations0020(self):
    self.runAltDestinationsX(20)

  def test_altDestinations0030(self):
    self.runAltDestinationsX(30)

  def test_altDestinations0040(self):
    self.runAltDestinationsX(40)

  def test_altDestinations0050(self):
    self.runAltDestinationsX(50)

  def test_altDestinations0060(self):
    self.runAltDestinationsX(60)

  def test_altDestinations0070(self):
    self.runAltDestinationsX(70)

  def test_altDestinations0080(self):
    self.runAltDestinationsX(80)

  def test_altDestinations0090(self):
    self.runAltDestinationsX(90)

  def test_altDestinations0100(self):
    self.runAltDestinationsX(100)

  def test_altDestinations0111(self):
    self.runAltDestinationsX(111)

  def test_altDestinations0112(self):
    try:
      self.runAltDestinationsX(112)
    except AssertionError as inst:
      self.deleteFile(self.schedulesFolder + 'altDestinations-112.dot')
      self.assertTrue('wrong return code 250' in inst.args[0], 'wrong error')

  def test_altDestinations1000(self):
    try:
      self.runAltDestinationsX(1000)
    except AssertionError as inst:
      self.assertEqual(inst.args[0], '1000 not greater than 1000 : Number of messages (1000) should be less than 1000.', 'wrong error')

"""Class UnitTestAltDestinations tests the limit of 9 altdst edges per block.
"""
class UnitTestAltDestinations(dm_testbench.DmTestbench):

  def test_altDestinationsOk(self):
    self.startPattern('altdst-flow-9.dot')
    self.checkRunningThreadsCmd()

  def test_altDestinationsOkSwitch(self):
    self.startPattern('altdst-9.dot')
    fileName = 'snoop_altDestinationsOkSwitch.csv'
    column_EVTNO = 8
    listSwitch = [(0, '0a'), (1, '0b'), (2, '0c'), (3, '0d'), (4, '0e'), (5, '0f'), (6, '10'), (7, '11'), (8, '12')]
    self.snoopToCsvWithAction(fileName, self.switchAction, actionArgs=[listSwitch], duration=len(listSwitch))
    checkList = {}
    for x, y in listSwitch:
      checkList['0x00' + y] = '>0'
    self.analyseFrequencyFromCsv(fileName, column_EVTNO, checkValues=checkList)
    self.deleteFile(fileName)

  def switchAction(self, argsList):
    listSwitch = argsList[0]
    for x, y in listSwitch:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'switch', 'Block', 'Msg0' + str(x)), [0], 0, 0)
      self.checkRunningThreadsCmd(0.1)

  def test_alt10DestinationsFlow(self):
    fileName = self.schedulesFolder + 'altdst-flow-10.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)

  def test_alt10Destinations(self):
    fileName = self.schedulesFolder + 'altdst-10.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)

  def test_altdst_missing_node(self):
    """DEV="dev/ttyUSB0"
      C="dm-cmd $DEV"
      S="dm-sched $DEV"

      $C halt
      $S clear
      $S add test4missing_alt.dot
      $S dump
      sleep 1.0
      $C -i cmd_test4missing_alt.dot
      $S dump
    """
    fileName = self.schedulesFolder + 'altdst-missing-node.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.delay(1.0)
    cmdFileName = self.schedulesFolder + 'altdst-missing-node-cmd.dot'
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i',
        cmdFileName), [0], linesCout=1, linesCerr=0)

