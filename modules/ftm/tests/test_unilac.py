import dm_testbench

"""Class tests the message performance for UNILAC
saft-ctl tr0 -xv snoop 0x1000001000000000 0xFFFFFFFF00000000 0 1 | wc -l
51
saft-ctl tr0 -xv snoop 0x1000257000000000 0xFFFFFFFF00000000 0 1 | wc -l
51

"""
class UnitTestUnilac(dm_testbench.DmTestbench):

  def unilacPerformance(self, numberOfMessages):
    """ Test performance of one CPU for UNILAC."""
    self.scheduleName = self.schedulesFolder + f'unilac{numberOfMessages}.dot'
    snoopFileName = f'snoop-unilac{numberOfMessages}.csv'
    self.patternName = f'UNILAC{numberOfMessages}'
    self.generateScheduleMsg(self.scheduleName, self.patternName, numberOfMessages)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        self.scheduleName), [0], linesCout=0, linesCerr=0)
    self.snoopToCsvWithAction(snoopFileName, self.actionUnilacPerformance, eventId='0x1000001000000000', mask='0xFFFFFFF000000000', duration=3)
    self.analyseFrequencyFromCsv(snoopFileName, column=20, printTable=True, checkValues={'0x0000000000000001': '>49'})
    self.deleteFile(self.scheduleName)
    self.deleteFile(snoopFileName)

  def actionUnilacPerformance(self):
    self.delay(0.3)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern',
        self.patternName), [0], linesCout=1, linesCerr=0)
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern',
        self.patternName), [0], linesCout=0, linesCerr=0)


  def testUnilac100(self):
    """ Test performance of one CPU for UNILAC. Should reach 5kHz timing messages (100 messages per 20 msec)."""
    self.unilacPerformance(100)

  def testUnilac900(self):
    """ Test performance of one CPU for UNILAC. Should reach 45kHz timing messages (900 messages per 20 msec)."""
    self.unilacPerformance(900)

  def testUnilac1800(self):
    """ Test performance of one CPU for UNILAC. Should reach 90kHz timing messages (1800 messages per 20 msec)."""
    self.unilacPerformance(1800)

  def generateScheduleMsg(self, fileName, patternName, numberOfMsgs, cpu=0):
    """Generate a schedule and write it to a file. The schedule has one block, a flow and timing messages.
    The timing messages are in two loops. At the end of the first loop, the flow switches flow to the second loop.
    The timing messages have a name counting from 0 to numberOfMsgs - 1. toffs is the number in
    the name multiplied by offset.

    :param fileName: the name of the schedule file
    :param patternName: the name of the pattern used for all nodes
    :param numberOfMsgs: the number of timing message nodes, maximal 1800.
    :param cpu: the CPU to use (Default value = 0)

    """
    self.assertGreater(1801, numberOfMsgs, f'Number of messages ({numberOfMsgs}) should be less than 1801.')
    # time period for 500Hz
    period = int(1000 * 1000 * 1000 / 50)
    offset = int(period / numberOfMsgs)
    period = int(period / 2)
    limit1 = int(numberOfMsgs / 2)
    # ~ print(f'{period=:d} {numberOfMsgs=:d} {offset=:d}')
    lines = []
    lines.append('digraph UnilacMsg {')
    lines.append(f'node [cpu={cpu} type=tmsg pattern={patternName} fid=1]')
    # create the nodes
    lines.append(f'Block{cpu}_0 [type=block patentry=1 patexit=1 qlo=1 tperiod={period:d}]')
    lines.append(f'Flow{cpu}_0 [type=flow prio=0 qty=1 toffs={int(period - 0.5*offset):d} tvalid=0 vabs=0]')
    for i in range(limit1):
      lines.append(f'Msg{cpu}_{i:04d} [par={i} evtno={i} toffs={offset*i:d}]')
    for i in range(limit1,numberOfMsgs):
      lines.append(f'Msg{cpu}_{i:04d} [par={i} evtno={i} toffs={offset*(i - limit1):d}]')
    # create the edges
    lines.append(f'Block{cpu}_0 -> Msg{cpu}_0000 [type=defdst]')
    for i in range(1,limit1):
      lines.append(f'Msg{cpu}_{i-1:04d} -> Msg{cpu}_{i:04d} [type=defdst]')
    lines.append(f'Msg{cpu}_{limit1-1:04d} -> Flow{cpu}_0 [type=defdst]')
    lines.append(f'Flow{cpu}_0 -> Block{cpu}_0 [type=defdst]')
    lines.append(f'Flow{cpu}_0 -> Block{cpu}_0 [type=target]')
    lines.append(f'Flow{cpu}_0 -> Msg{cpu}_{limit1:04d} [type=flowdst]')
    lines.append(f'Block{cpu}_0 -> Msg{cpu}_{limit1:04d} [type=altdst]')
    for i in range(limit1+1, numberOfMsgs):
      lines.append(f'Msg{cpu}_{i-1:04d} -> Msg{cpu}_{i:04d} [type=defdst]')
    lines.append(f'Msg{cpu}_{numberOfMsgs-1:04d} -> Block{cpu}_0 [type=defdst]')
    lines.append('}')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))
