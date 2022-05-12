import dm_testbench
import pytest

"""
Class collects unit tests for startthread nodes and origin nodes.
"""
class UnitTestBoosterStartThread(dm_testbench.DmTestbench):

  def test_threeThreads0(self):
    self.startPattern('booster_startthread-0.dot', 'MAIN')
    file_name = 'snoop_startthread-0.csv'
    self.snoopToCsv(file_name, 1)
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO,
        checkValues={'0x0000': '>9', '0x0002': '>20', '0x0003': '>10'})
    self.deleteFile(file_name)

  def test_threeThreads1(self):
    self.startPattern('booster_startthread-1.dot', 'BOOST_REQ')
    file_name = 'snoop_startthread-1.csv'
    self.snoopToCsv(file_name, 2)
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO,
        checkValues={'0x0100': '>7', '0x0200': '>0', '0x0102': '=1', '0x0103': '=1', '0x0160': '>0'})
    self.deleteFile(file_name)

  def test_threeThreads2(self):
    self.startPattern('booster_startthread-2.dot', 'BOOST_REQ')
    file_name = 'snoop_startthread-2.csv'
    self.snoopToCsv(file_name, 3)
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO,
        checkValues={'0x0100': '>7', '0x0200': '>0', '0x0102': '=1', '0x0103': '1', '0x0160': '>0'})
    self.deleteFile(file_name)

  def test_threeThreads3(self):
    self.startPattern('booster_startthread-3.dot', 'MAIN')
    file_name = 'snoop_startthread-3.csv'
    self.snoopToCsv(file_name, 3)
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO,
        checkValues={'0x0001': '>0', '0x0002': '>0', '0x0003': '>0'})
    self.deleteFile(file_name)

  def test_booster_all_threads(self):
    self.startPattern('booster-all-threads.dot', 'MAIN')
    file_name = 'snoop_all_threads.csv'
    self.snoopToCsv(file_name, 1)
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO, checkValues={'0x0001': '>9'})
    self.analyseDmCmdOutput(0xFF)
    self.deleteFile(file_name)

  def test_booster_thread_0_loop(self):
    self.startPattern('booster-thread-0-loop.dot', 'MAIN')
    file_name = 'snoop_booster-thread-0-loop.csv'
    column_Param = 20
    self.snoopToCsv(file_name, 3)
    self.analyseFrequencyFromCsv(file_name, column_Param)
    # assert that there are more than 61 tmsg in 3 seconds with EVTNO 0x0001.
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO, checkValues={'0x0001': '>60'})
    self.deleteFile(file_name)

  def test_booster_thread_0(self):
    self.startPattern('booster-thread-0.dot', 'MAIN')
    file_name = 'snoop_booster-thread-0.csv'
    self.snoopToCsv(file_name, 3)
    parameter_column = 20
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    # assert that there are more than 2999 tmsg in 3 seconds with EVTNO 0x0001.
    column_EVTNO = 8
    self.analyseFrequencyFromCsv(file_name, column_EVTNO, checkValues={'0x0001': '>2990'})
    self.deleteFile(file_name)
