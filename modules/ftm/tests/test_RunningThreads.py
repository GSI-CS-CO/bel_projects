import dm_testbench    # contains super class
import logging
import os
import datetime
import pytest

"""
Module collects tests for running threads.

Run with (example):
OPTIONS='-k test_RunningThreads.py -rP --log-level=WARNING --development' make remote
"""
class RunningThreads(dm_testbench.DmTestbench):

  @pytest.mark.development
  def testLoopRunningThreads(self):
    """Prepare all threads on all CPUs.
    """
    self.testName = os.environ['PYTEST_CURRENT_TEST']
    logging.getLogger().debug(f'{self.testName} prepare threads {datetime.datetime.now()}')
    cpus = 15
    self.loop = 1000
    self.durations = []
    for count in range(self.loop):
      start = datetime.datetime.now()
      try:
        self.prepareRunThreads(cpus)
        self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, 'reset'), [0], 1, 0)
        self.durations.append(datetime.datetime.now() - start)
        logging.getLogger().info(f'{count=}, start={str(start)}, {self.durations[count]}')
      finally:
        if count > len(self.durations) -1:
          logging.getLogger().info(f'{count=}, start={str(start)}, duration={str(datetime.datetime.now() - start)}')

  def tearDown(self):
    super().tearDown()
    if len(self.durations) > 0:
      averageDuration = sum(self.durations, datetime.timedelta(0)) / len(self.durations)
      maxDuration = max(self.durations)
      minDuration = min(self.durations)
    else:
      averageDuration = 0.0
      maxDuration = 0.0
      minDuration = 0.0
    logging.getLogger().info(f'loops={self.loop}, {len(self.durations)}, average={averageDuration}, max={maxDuration}, min={minDuration}, max - min={maxDuration-minDuration} ')
    logging.getLogger().debug(f'{self.testName}         threads {datetime.datetime.now()}')
