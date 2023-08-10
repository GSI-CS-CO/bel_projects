import dm_testbench
import pytest

"""Class UnitTestAltDestinations tests the limit of 9 altdst edges per block.
"""
class UnitTestAltDestinations(dm_testbench.DmTestbench):

  def test_altDestinationsOk(self):
    self.startPattern('altdst-flow-9.dot')
    self.checkRunningThreadsCmd()

  def test_altDestinationsOkSwitch(self):
    self.startPattern('altdst-9.dot')
    file_name = 'snoop_altDestinationsOkSwitch.csv'
    column_EVTNO = 8
    listSwitch = [(0, '0a'), (1, '0b'), (2, '0c'), (3, '0d'), (4, '0e'), (5, '0f'), (6, '10'), (7, '11'), (8, '12')]
    for x, y in listSwitch:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'switch', 'Block', 'Msg0' + str(x)), [0], 0, 0)
      self.checkRunningThreadsCmd(0.1)
      self.snoopToCsv(file_name, duration=1)
      self.analyseFrequencyFromCsv(file_name, column_EVTNO, checkValues={'0x00' + y: '>0'})
    self.deleteFile(file_name)

  def test_altDestinationsFlowFail(self):
    fileName = self.schedules_folder + 'altdst-flow-10.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)

  def test_altDestinationsFail(self):
    fileName = self.schedules_folder + 'altdst-10.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)

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
    fileName = self.schedules_folder + 'altdst-missing-node.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [0], linesCout=0, linesCerr=0)
    self.delay(1.0)
    cmdFileName = self.schedules_folder + 'altdst-missing-node-cmd.dot'
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, '-i',
        cmdFileName), [0], linesCout=1, linesCerr=0)

