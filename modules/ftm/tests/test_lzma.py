import dm_testbench
import pytest

"""Class tests the error in decompression which occurs with large
schedules and log pattern names.
"""
class UnitTestLzma(dm_testbench.DmTestbench):

  def test_large_patternname_ok(self):
    """OK test: use a schedule with 862 messages and a pattern name of 30 chars.
    This works without an exception, including start of pattern.
    """
    fileName = self.schedules_folder + 'lzma_862_30_msg.dot'
    patternName = 'PatternNameMsg4567890123456789'
    self.generate_schedule_msg(fileName, patternName, 862)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.deleteFile(fileName)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', patternName),
        [0], linesCout=1, linesCerr=0)

  def test_large_patternname_fail(self):
    """Fail test: use a schedule with 1000 messages and a pattern name of 30 chars.
    This does not work. Ends with SEGV (return code -11).
    """
    fileName = self.schedules_folder + 'lzma_1000_30_msg.dot'
    self.generate_schedule_msg(fileName, 'PatternNameMsg4567890123456789', 1000)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [-11], linesCout=0, linesCerr=0)
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
