import dm_testbench

"""
Start a pps pattern.

Required: set up of DmTestbench class.
"""
class DmPps(dm_testbench.DmTestbench):

  def test_pps(self):
    snoopFile = 'snoop_pps.csv'
    self.startPattern('pps.dot')
    self.snoopToCsv(snoopFile, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x00d7 and 0x00cd occur.
    self.analyseFrequencyFromCsv(snoopFile, column=8, printTable=True, checkValues={'0x00d7': '>0', '0x00cd': '>0'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x234 and 0x123 occur.
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000234': '>0', '0x0000000000000123': '>0'})
    self.deleteFile(snoopFile)

  def test_ppsAdd(self):
    snoopFile = 'snoop_ppsAdd.csv'
    scheduleFile = 'pps-subgraph.dot'
    downloadFile = 'pps-download.dot'
    self.startPattern(scheduleFile)
    self.snoopToCsv(snoopFile, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x0fff occur.
    self.analyseFrequencyFromCsv(snoopFile, column=8, printTable=True, checkValues={'0x0fff': '>1'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x400 occur.
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000000400': '>1'})
    self.deleteFile(snoopFile)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile, downloadFile))
    self.deleteFile(downloadFile)

  def testPpsAdd0(self):
    self.runPpsAdd(0)

  def testPpsAdd1(self):
    number = 1
    # ~ snoopFile0 = f'snoop_test{number}_0.csv'
    # ~ snoopFile1 = f'snoop_test{number}_1.csv'
    scheduleFile0 = f'pps-test{number}-0.dot'
    scheduleFile1 = f'pps-test{number}-1.dot'
    downloadFile0 = f'pps-test{number}-0-download.dot'
    downloadFile1 = f'pps-test{number}-1-download.dot'
    self.startPattern(scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startPattern(scheduleFile1)
    # ~ self.snoopToCsv(snoopFile1, duration=2)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile1.replace('.dot', '-download.dot'), downloadFile1))
    self.deleteFile(downloadFile1)
    # ~ self.snoopToCsvWithAction(snoopFile1, self.actionPpsAdd0, duration=10)
    # ~ self.analyseFrequencyFromCsv(snoopFile1, column=8, printTable=True, checkValues={'0x0fff': '>8'})
    # ~ self.deleteFile(snoopFile1)

  def testPpsAdd2(self):
    number = 2
    # ~ snoopFile0 = f'snoop_test{number}_0.csv'
    # ~ snoopFile1 = f'snoop_test{number}_1.csv'
    scheduleFile0 = f'pps-test{number}-0.dot'
    scheduleFile1 = f'pps-test{number}-1.dot'
    downloadFile0 = f'pps-test{number}-0-download.dot'
    downloadFile1 = f'pps-test{number}-1-download.dot'
    self.startPattern(scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startPattern(scheduleFile1)
    # ~ self.snoopToCsv(snoopFile1, duration=2)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile1.replace('.dot', '-download.dot'), downloadFile1))
    self.deleteFile(downloadFile1)
    # ~ self.snoopToCsvWithAction(snoopFile1, self.actionPpsAdd0, duration=10)
    # ~ self.analyseFrequencyFromCsv(snoopFile1, column=8, printTable=True, checkValues={'0x0fff': '>8'})
    # ~ self.deleteFile(snoopFile1)

  def testPpsAdd3(self):
    number = 3
    scheduleFile0 = f'pps-test{number}-0.dot'
    scheduleFile1 = f'pps-test{number}-1.dot'
    scheduleFile2 = f'pps-test{number}-2.dot'
    scheduleFile3 = f'pps-test{number}-3.dot'
    scheduleFile4 = f'pps-test{number}-4.dot'
    downloadFile0 = f'pps-test{number}-0-download.dot'
    downloadFile1 = f'pps-test{number}-1-download.dot'
    downloadFile2 = f'pps-test{number}-2-download.dot'
    downloadFile3 = f'pps-test{number}-3-download.dot'
    downloadFile4 = f'pps-test{number}-4-download.dot'
    self.startPattern(scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startPattern(scheduleFile1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile1.replace('.dot', '-download.dot'), downloadFile1))
    self.deleteFile(downloadFile1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile2])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile2])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile2.replace('.dot', '-download.dot'), downloadFile2))
    self.deleteFile(downloadFile2)
    self.startPattern(scheduleFile3)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile3])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile3.replace('.dot', '-download.dot'), downloadFile3))
    self.deleteFile(downloadFile3)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile4])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile4])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile4.replace('.dot', '-download.dot'), downloadFile4))
    self.deleteFile(downloadFile4)

  def testPpsAdd4(self):
    number = 4
    scheduleFile0 = f'pps-test{number}-0.dot'
    scheduleFile1 = f'pps-test{number}-1.dot'
    scheduleFile2 = f'pps-test{number}-2.dot'
    downloadFile0 = f'pps-test{number}-0-download.dot'
    downloadFile1 = f'pps-test{number}-1-download.dot'
    downloadFile2 = f'pps-test{number}-2-download.dot'
    self.startPattern(scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile1], expectedReturnCode=[250])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile1.replace('.dot', '-download.dot'), downloadFile1))
    self.deleteFile(downloadFile1)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'stoppattern', 'A'))
    self.delay(0.5)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile2], expectedReturnCode=[0])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile2])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile2.replace('.dot', '-download.dot'), downloadFile2))
    self.deleteFile(downloadFile2)

  def testPpsAdd5(self):
    number = 5
    scheduleFile0 = f'pps-test{number}-0.dot'
    scheduleFile1 = f'pps-test{number}-1.dot'
    downloadFile0 = f'pps-test{number}-0-download.dot'
    downloadFile1 = f'pps-test{number}-1-download.dot'
    self.startPattern(scheduleFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile1], expectedReturnCode=[0])
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile1.replace('.dot', '-download.dot'), downloadFile1))
    self.deleteFile(downloadFile1)

  def runPpsAdd(self, number):
    snoopFile0 = f'snoop_test{number}_0.csv'
    snoopFile1 = f'snoop_test{number}_1.csv'
    scheduleFile0 = f'pps-test{number}-0.dot'
    scheduleFile1 = f'pps-test{number}-1.dot'
    downloadFile0 = f'pps-test{number}-0-download.dot'
    downloadFile1 = f'pps-test{number}-1-download.dot'
    self.startPattern(scheduleFile0)
    # ~ self.snoopToCsv(snoopFile0, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x0fff occur.
    # ~ self.analyseFrequencyFromCsv(snoopFile0, column=8, printTable=True, checkValues={'0x0fff': '>1'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x400 occur.
    # ~ self.analyseFrequencyFromCsv(snoopFile0, column=20, printTable=True, checkValues={'0x0000000000000400': '>1'})
    # ~ self.deleteFile(snoopFile0)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile0])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile0, downloadFile0))
    self.deleteFile(downloadFile0)
    self.startPattern(scheduleFile1)
    self.snoopToCsv(snoopFile1, duration=2)
    # analyse column 8 which contains the evtno.
    # check that evtno 0x0fff occur.
    # ~ self.analyseFrequencyFromCsv(snoopFile1, column=8, printTable=True, checkValues={'0x0fff': '>1'})
    # analyse column 20 which contains the parameter.
    # check that parameter 0x400 occur.
    # ~ self.analyseFrequencyFromCsv(snoopFile1, column=20, printTable=True, checkValues={'0x0000000000000400': '>1'})
    # ~ self.deleteFile(snoopFile1)
    self.startAndCheckSubprocess([self.binaryDmSched, self.datamaster, 'status', '-o', downloadFile1])
    self.startAndCheckSubprocess(('scheduleCompare', '-u', self.schedulesFolder + scheduleFile1.replace('.dot', '-download.dot'), downloadFile1))
    self.deleteFile(downloadFile1)
    self.snoopToCsvWithAction(snoopFile1, self.actionPpsAdd0, duration=10)
    self.analyseFrequencyFromCsv(snoopFile1, column=8, printTable=True, checkValues={'0x0fff': '>8'})
    self.deleteFile(snoopFile1)

  def actionPpsAdd0(self):
    for i in range(10):
      self.startAndCheckSubprocess([self.binaryDmCmd, self.datamaster, 'flow', 'B_A', 'Evt_A'])
      self.delay(0.9)
