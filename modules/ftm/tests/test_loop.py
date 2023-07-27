import dm_testbench
import pytest

"""Class collects unit tests for loop.
"""
class UnitTestLoop(dm_testbench.DmTestbench):

  @pytest.mark.thread8
  def test_dynamic_loop(self):
    """Load the schedule dynamic-loop-schedule.dot and start pattern IN_A.
    Check the visited nodes (rawvisited) and rawstatus.
    Then start pattern IN_B and check the visited nodes (rawvisited) and rawstatus again.
    """
    self.startPattern('dynamic-loop-schedule.dot', 'IN_A')
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-0-1.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_B'))
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-1-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-1-1.txt')

  @pytest.mark.thread32
  def test_dynamic_loop_thread32(self):
    """Load the schedule dynamic-loop-schedule.dot and start pattern IN_A.
    Check the visited nodes (rawvisited) and rawstatus.
    Then start pattern IN_B and check the visited nodes (rawvisited) and rawstatus again.
    """
    self.startPattern('dynamic-loop-schedule.dot', 'IN_A')
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-0-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-0-1-thread32.txt')
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'IN_B'))
    self.delay(0.5)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmSched, self.datamaster, 'rawvisited'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-1-0.txt')
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'rawstatus'))
    self.compareExpectedOutput(stdoutLines, self.schedules_folder + 'dynamic-loop-expected-1-1-thread32.txt')
