import dm_testbench
import pytest

"""Class UnitTestAltDestinations tests the limit of 10 altdst edges per block.
"""
class UnitTestAltDestinations(dm_testbench.DmTestbench):

  def test_altDestinationsOk(self):
    self.startPattern('altdst-flow-10.dot')
    self.checkRunningThreadsCmd()

  def test_altDestinationsOkSwitch(self):
    self.startPattern('altdst-10.dot')
    file_name = 'snoop_altDestinationsOkSwitch.csv'
    column_EVTNO = 8
    listSwitch = [(0, '0a'), (1, '0b'), (2, '0c'), (3, '0d'), (4, '0e'), (5, '0f'), (6, '10'), (7, '11'), (8, '12'), (9, '13')]
    for x, y in listSwitch:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'switch', 'Block', 'Msg0' + str(x)), [0], 0, 0)
      self.checkRunningThreadsCmd(0.1)
      self.snoopToCsv(file_name, 1)
      self.analyseFrequencyFromCsv(file_name, column_EVTNO, checkValues={'0x00' + y: '>0'})
    self.deleteFile(file_name)

  def test_altDestinationsFlowFail(self):
    fileName = self.schedules_folder + 'altdst-flow-11.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)

  def test_altDestinationsFail(self):
    fileName = self.schedules_folder + 'altdst-11.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add',
        fileName), [250], linesCout=2, linesCerr=3)

