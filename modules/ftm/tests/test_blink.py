import dm_testbench

"""
Start blink schedule.

Required: set up of DmTestbench class.
"""
class DmBlink(dm_testbench.DmTestbench):

  def test_blink(self):
    schedule = 'blink.dot'
    fileName = 'snoop_blink.csv'
    self.addSchedule(schedule)
    self.snoopToCsvWithAction(fileName, self.doAction, 10)
    # ~ # analyse column 8 which contains the evtno.
    # ~ # check that evtno 0x0110 and 0x0112 occur.
    self.analyseFrequencyFromCsv(fileName, column=8, printTable=True, checkValues={'0x0001': '=1', '0x0110': '>0', '0x0112': '>0'})
    # ~ # analyse column 8 which contains the parameter.
    # ~ # check that paramter 0x123, 0x456, 0x789, and 0xABC occur.
    self.analyseFrequencyFromCsv(fileName, column=20, printTable=True, checkValues={'0x0000000000000001': '=1', '0x0000000000000123': '>0', '0x0000000000000456': '>0', '0x0000000000000789': '>0', '0x0000000000000abc': '>0'})
    self.deleteFile(fileName)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'preptime', '-c0',  '-t0', '1000000'), linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'preptime', '-c0', '-t1', '1000000'), linesCout=0, linesCerr=0)

  def doAction(self):
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'ping'), linesCout=1, linesCerr=0)
    self.delay(0.5)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'origin', '-c0', '-t0', 'Evt_POLICE0'), linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'origin', '-c0', '-t1', 'Evt_FIREF0'), linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'preptime', '-c0',  '-t0', '1000000000'), linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'preptime', '-c0', '-t1', '1000000000'), linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'starttime', '-c0', '-t0', '0'), linesCout=0, linesCerr=0)
    # ~ self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'starttime', '-c0', '-t1', '1000'), linesCout=0, linesCerr=0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'starttime', '-c0', '-t1', '0'), linesCout=0, linesCerr=0)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'origin', '-c0', '-t0'), linesCout=1, linesCerr=0)
    print(stdoutLines)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'origin', '-c0', '-t1'), linesCout=1, linesCerr=0)
    print(stdoutLines)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'preptime', '-c0',  '-t0'), linesCout=1, linesCerr=0)
    print(stdoutLines)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'preptime', '-c0', '-t1'), linesCout=1, linesCerr=0)
    print(stdoutLines)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'starttime', '-c0', '-t0'), linesCout=1, linesCerr=0)
    print(stdoutLines)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'starttime', '-c0', '-t1'), linesCout=1, linesCerr=0)
    print(stdoutLines)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'start', '0x3'), linesCout=0, linesCerr=0)
    stdoutLines = self.startAndGetSubprocessStdout((self.binaryDmCmd, self.datamaster, 'heap'), linesCout=9, linesCerr=0)
