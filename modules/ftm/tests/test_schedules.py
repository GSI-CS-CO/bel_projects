import dm_testbench
import pytest

"""
Test two schedules from production.
Start all pattern in the schedule and analyse the frequency of timing messages for some seconds.

For the long running tests (1 minute, 10 minutes) both markers --runslow and --development
have to be set on command line.
"""
class Schedules(dm_testbench.DmTestbench):

  def runFrequencySchedule(self, duration=6):
    """Run a schedule from production (SIS18, Jun2 2020). This schedule
    has 544 nodes and 607 edges. Start all pattern, snoop for the given duration
    and check for two evtno values.
    """
    scheduleFile = 'schedule1.dot'
    self.startAllPattern(scheduleFile)
    snoopFileName = 'snoop_schedule1.csv'
    parameterColumn = 8
    self.snoopToCsv(snoopFileName, duration=duration)
    self.analyseFrequencyFromCsv(snoopFileName, parameterColumn, checkValues={'0x0100': '>30', '0x021b': '>0'}, addDelayed=True)
    self.deleteFile(snoopFileName)
    # compare downloaded schedule with original schedule.
    statusFile = scheduleFile.replace('.dot', '-download.dot')
    statusFile1 = scheduleFile.replace('.dot', '-download1.dot')
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', statusFile))
    try:
      self.startAndCheckSubprocess(('scheduleCompare', '-s', self.schedulesFolder + statusFile, statusFile), [0], 0, 0)
    except AssertionError as instance:
      self.assertEqual(instance.args[0], "False is not true : wrong return code 1, expected: [0], Command line: ('scheduleCompare', '-s', 'schedules/schedule1-download.dot', 'schedule1-download.dot')\nstderr: []\nstdout: []", 'wrong exception')
      self.startAndCheckSubprocess(('scheduleCompare', '-s', self.schedulesFolder + statusFile1, statusFile), [0], 0, 0)
    self.deleteFile(statusFile)

  def testFrequencySchedule1(self):
    """Run the snoop for 16 seconds.
    """
    self.runFrequencySchedule(16)

  def testFrequencySchedule2(self):
    """Run a schedule from production (SIS18, March 2021). This schedule
    has 879 nodes and 987 edges. Start all pattern, snoop for the given duration
    and check for two evtno values.
    """
    if self.cpuQuantity > 3:
      self.startAllPattern('schedule2.dot')
      snoopFileName = 'snoop_schedule2.csv'
      parameterColumn = 8
      self.snoopToCsv(snoopFileName, duration=6)
      self.analyseFrequencyFromCsv(snoopFileName, parameterColumn, checkValues={'0x0100': '>15', '0x021b': '>0'})
      self.deleteFile(snoopFileName)

  def testSchedule2024_10_02_int(self):
    """Run a schedule from integration (2024-10-02). Start all pattern, snoop for the given duration
    and check for two evtno values.
    """
    if self.cpuQuantity > 3:
      self.startAllPattern('2024-10-02-schedule-int.dot')
      snoopFileName = '2024-10-02-schedule-int.csv'
      parameterColumn = 8
      self.snoopToCsv(snoopFileName, duration=6)
      self.analyseFrequencyFromCsv(snoopFileName, parameterColumn, checkValues={'0x0100': '>0', '0x021b': '>0'})
      self.deleteFile(snoopFileName)

  def testSchedule2024_10_02_prod(self):
    """Run a schedule from production (2024-10-02). Start all pattern, snoop for the given duration
    and check for two evtno values.
    """
    if self.cpuQuantity > 3:
      self.startAllPattern('2024-10-02-schedule-prod.dot')
      snoopFileName = '2024-10-02-schedule-prod.csv'
      parameterColumn = 8
      self.snoopToCsv(snoopFileName, duration=6)
      self.analyseFrequencyFromCsv(snoopFileName, parameterColumn, checkValues={'0x0100': '>0', '0x0200': '>0'})
      self.deleteFile(snoopFileName)

  @pytest.mark.slow
  @pytest.mark.development
  def testFrequencySchedule1_0060(self):
    """Run the snoop for 1 minute.
    """
    self.runFrequencySchedule(60)

  @pytest.mark.slow
  @pytest.mark.development
  def testFrequencySchedule1_0600(self):
    """Run the snoop for 10 minutes.
    """
    self.runFrequencySchedule(600)
