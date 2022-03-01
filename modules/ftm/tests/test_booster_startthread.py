import dm_testbench
import pytest

"""
Class collects unit tests for startthread nodes and origin nodes.
"""
class UnitTestBoosterStartThread(dm_testbench.DmTestbench):

  def test_threeThreads(self):
    self.startPattern('booster_startthread.dot', 'MAIN')
    file_name = 'snoop_startthread.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 3)
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0001', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0002', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0003', file_name), [0])
    self.deleteFile(file_name)

  def test_threeThreads3(self):
    self.startPattern('booster_startthread-3.dot', 'MAIN')
    file_name = 'snoop_startthread-3.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 3)
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0001', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0002', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0003', file_name), [0])
    self.deleteFile(file_name)

  @pytest.mark.slow
  def test_threeThreads1(self):
    self.startPattern('booster_startthread-1.dot', 'BOOST_REQ')
    file_name = 'snoop_startthread-1.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 15)
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0100', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0200', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0102', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0103', file_name), [0])
    self.deleteFile(file_name)

  @pytest.mark.slow
  def test_threeThreads2(self):
    self.startPattern('booster_startthread-2.dot', 'BOOST_REQ')
    file_name = 'snoop_startthread-2.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 10)
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0100', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0200', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0102', file_name), [0])
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0103', file_name), [0])
    self.deleteFile(file_name)

  def test_booster_all_threads(self):
    self.startPattern('booster-all-threads.dot', 'MAIN')
    file_name = 'snoop_all_threads.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 3)
    self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0001', file_name), [0])
    self.deleteFile(file_name)

  def test_booster_thread_0_loop(self):
    self.startPattern('booster-thread-0-loop.dot', 'MAIN')
    file_name = 'snoop_booster-thread-0-loop.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 3)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    output = self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0001', file_name), [0])
    self.assertGreater(len(output), 61)
    self.deleteFile(file_name)

  def test_booster_thread_0(self):
    self.startPattern('booster-thread-0.dot', 'MAIN')
    file_name = 'snoop_booster-thread-0.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 3)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    output = self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0001', file_name), [0])
    self.assertGreater(len(output), 2999)
    self.deleteFile(file_name)
