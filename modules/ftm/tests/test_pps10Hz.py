import dm_testbench

"""
Start a loop pattern with x Hz event messages.
Mainly used to test the testbench.
"""
class DmPpsXHz(dm_testbench.DmTestbench):

  def test_10Hz(self):
    """ Test with 10 timing messages in a second."""
    self.runXhz(10)

  def test_100Hz(self):
    """ Test with 100 timing messages in a second."""
    self.runXhz(100)

  def test_1000Hz(self):
    """ Test with 1000 timing messages in a second."""
    self.runXhz(1000)

  def runXhz(self, numberOfMessages):
    """ Test with a loop pattern with x Hz event messages."""
    scheduleName = f'pps{numberOfMessages}Hz.dot'
    snoopFileName = f'snoop_pps{numberOfMessages}Hz.csv'
    patternName = f'PPS{numberOfMessages}Hz'
    self.generate_schedule_msg(self.schedulesFolder + scheduleName, patternName, numberOfMessages)
    self.startPattern(scheduleName, patternName)
    self.snoopToCsv(snoopFileName, duration=2)
    keyList = {'0x0000000000000000': '>0', }
    for i in range(1, numberOfMessages):
      keyList[f'0x{i:016x}'] = '>0'
    self.analyseFrequencyFromCsv(snoopFileName, column=20, printTable=True, checkValues=keyList)
    self.deleteFile(self.schedulesFolder + scheduleName)
    self.deleteFile(snoopFileName)

  def generate_schedule_msg(self, fileName, patternName, numberOfMsgs, cpu=0):
    """Generate a schedule and write it to a file. The schedule has one
    block and numberOfMsgs timing messages. The block is 1 second long.
    The timing messages are in one loop. The timing messages have a name
    counting from 0 to numberOfMsgs - 1. toffs is the number in the name
    multiplied by offset. This produces timing messages at numberOfMsgs Hz.
    Range: 1 to 1000.

    :param fileName: the name of the schedule file
    :param patternName: the name of the pattern used for all nodes
    :param numberOfMsgs: the number of timing message nodes, maximal 1800.
    :param cpu: the CPU to use (Default value = 0)
    """

    self.assertGreater(1001, numberOfMsgs, f'Number of messages ({numberOfMsgs}) should be at most 1000.')
    self.assertGreater(numberOfMsgs, 0, f'Number of messages ({numberOfMsgs}) should be at least 1.')
    # time period for 1Hz
    period = int(1000 * 1000 * 1000)
    offset = int(period / numberOfMsgs)
    # ~ print(f'{period=:d} {numberOfMsgs=:d} {offset=:d}')
    lines = []
    lines.append(f'digraph PPS{numberOfMsgs}Hz ' + '{')
    lines.append(f'node [cpu={cpu} type=tmsg toffs=0 tef=0 patentry=0 patexit=0 beamin=0 sid=2 bpid=8 reqnobeam=0 vacc=0 pattern={patternName} fid=1]')
    lines.append('edge [type=defdst]')
    # create the nodes
    lines.append(f'Block{cpu}_0 [type=block patentry=1 patexit=1 qlo=1 tperiod={period:d}]')
    for i in range(numberOfMsgs):
      lines.append(f'Msg{cpu}_{i:04d} [par={i} evtno={i} toffs={offset*i:d}]')
    # create the edges
    lines.append(f'Block{cpu}_0 -> Msg{cpu}_0000')
    for i in range(1,numberOfMsgs):
      lines.append(f'Msg{cpu}_{i-1:04d} -> Msg{cpu}_{i:04d}')
    lines.append(f'Msg{cpu}_{numberOfMsgs-1:04d} -> Block{cpu}_0')
    lines.append('}')
    # write the file
    with open(fileName, 'w') as file1:
      file1.write("\n".join(lines))
