import dm_testbench
import pytest

"""Class collects basic tests.
1. add an remove a schedule.
"""
class UnitTestBasic(dm_testbench.DmTestbench):

  @pytest.mark.thread8
  def test_static_basic(self):
    """Load the schedule static-basic-schedule.dot.
    """
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-0-0.txt')
    self.addSchedule('static-basic-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-1-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'remove', self.schedules_folder + 'static-basic-schedule.dot'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-0-0.txt')

  @pytest.mark.thread32
  def test_static_basic_thread32(self):
    """Load the schedule static-basic-schedule.dot.
    """
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-0-0.txt')
    self.addSchedule('static-basic-schedule.dot')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-1-0-thread32.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'remove', self.schedules_folder + 'static-basic-schedule.dot'))
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'status'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'static-basic-expected-0-0.txt')
