import dm_testbench
from datetime import datetime as dt

"""
Class collects unit tests for startthread nodes and origin nodes.
"""
class UnitTestBoosterStartThread(dm_testbench.DmTestbench):

  def test_threeThreads(self):
    self.startPattern('booster_startthread.dot', 'MAIN')
    file_name = 'snoop_startthread.csv'
    parameter_column = 20
    self.snoopToCsv(file_name, 3)
    print(self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0001', file_name), [0]))
    print(self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0002', file_name), [0]))
    print(self.startAndGetSubprocessStdout(('grep', 'EVTNO: 0x0003', file_name), [0]))
    self.deleteFile(file_name)
