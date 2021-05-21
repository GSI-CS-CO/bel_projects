import dm_testbench

"""
Start all pattern in a schedule and analyse the frequency of timing messages for 5 seconds.
"""
class Schedules(dm_testbench.DmTestbench):

  def test_frequency_schedule1(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8 #20
    self.snoopToCsv(file_name, 6)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def test_frequency_schedule2(self):
    self.startAllPattern(self.datamaster, 'schedule2.dot')
    file_name = 'snoop_schedule2.csv'
    parameter_column = 8 #20
    self.snoopToCsv(file_name, 6)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_frequency_schedule1_0060(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8
    self.snoopToCsv(file_name, 60)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_frequency_schedule1_0600(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8
    self.snoopToCsv(file_name, 600)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)

  def t1est_frequency_schedule1_3600(self):
    self.startAllPattern(self.datamaster, 'schedule1.dot')
    file_name = 'snoop_schedule1.csv'
    parameter_column = 8
    self.snoopToCsv(file_name, 3600)
    self.analyseFrequencyFromCsv(file_name, parameter_column)
    self.deleteFile(file_name)
