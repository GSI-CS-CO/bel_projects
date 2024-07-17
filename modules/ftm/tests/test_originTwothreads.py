import dm_testbench

"""
Tests for node types origin and startthread.
"""
class TestOriginTwoThreads(dm_testbench.DmTestbench):

  def startPatternAndSee(self):
    """Start pattern B on thread 1. Check that thread 1 is running.
    """
    self.delay(0.5)
    # start pattern B
    self.startPattern('twothreads.dot', 'B')
    # check that thread 1 is running
    self.analyseDmCmdOutput('01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'[:self.cpuQuantity*self.threadQuantity])

  def test_twoThreads(self):
    """Add the schedule. Trigger action and snoop.
    Check the parameter field of the timing messages.
    First message has parameter 1, second message has parameter 2, third and
    all following messages have parameter 1.
    If the first message is missing, this is not detected by the check. More than
    15 messages with parameter 1 are following. If snoop starts to late, it may
    not detect the second message and the check fails.
    """
    snoopFileName = 'snoop_twothreads.csv'
    self.snoopToCsvWithAction(snoopFileName, self.startPatternAndSee, duration=2)
    # analyse column 20 which contains the parameter.
    # check par=1:>15, par=2:>0 for snoop of 1 second.
    self.analyseFrequencyFromCsv(snoopFileName, column=20, printTable=True,
        checkValues={'0x0000000000000001': '>15', '0x0000000000000002': '>0'})
    self.deleteFile(snoopFileName)
