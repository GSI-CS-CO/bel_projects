import dm_testbench
import pytest

"""
Test cases for edges of type reference.
These edges have the additional attributes
fieldtail: offset into the source node, fieldhead: offset into
the target node, fieldwidth: width of the referenced field.
"""
class ReferenceEdge(dm_testbench.DmTestbench):

  def testReferenceEdgeSimple(self):
    """Use a schedule with an edge of type reference between two loops
    (a block and a tmsg). The loops run with 10Hz.
    Check for the correct parameter value when using the reference.
    """
    snoopFile = 'snoop_reference1.csv'
    self.scheduleFile0 = 'reference1.dot'
    if self.threadQuantity == 32:
      self.scheduleFile0 = self.scheduleFile0.replace('.dot', '-thread32.dot')
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionReference1, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000010000fa0': '>5', '0x10000fa000000000': '>5'})
    self.deleteFile(snoopFile)

  def actionReference1(self):
    """During snoop start pattern PAT2. This produces messages with 10Hz.
    The paramter of this tmsg T_PAT2 comes from the tmsg T_PAT1.
    Download the schedule for later compare.
    """
    self.startPattern(self.scheduleFile0, 'PAT2')
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)
    self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'startpattern', 'PAT1'), [0], 1, 0)

  def testReferenceEdgeLoop1(self):
    """Use a schedule with a loop of a block and two tmsg.
    The loops run with 1Hz. There is a reference between the two tmsg nodes.
    Check for the correct gid value in the timing messages.
    Check for the correct parameter value when using the reference.
    """
    snoopFile = 'snoop_reference_loop1.csv'
    self.scheduleFile0 = 'reference-loop1.dot'
    if self.threadQuantity == 32:
      self.scheduleFile0 = self.scheduleFile0.replace('.dot', '-thread32.dot')
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionReferenceLoop1, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.scheduleFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.analyseFrequencyFromCsv(snoopFile, column=6, printTable=True, checkValues={'0x0001': '>1', '0x0002': '>1'})
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000225002': '>3'})
    self.deleteFile(snoopFile)

  def actionReferenceLoop1(self):
    """During snoop start pattern ref1. This produces messages with 1Hz.
    Download the schedule for later compare.
    """
    self.startPattern(self.scheduleFile0, 'ref1')
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)

  def testReferenceEdgeLoop3(self):
    """Use a schedule with a loop of a block and four tmsg.
    The loops run with 1Hz. There is are references between the tmsg nodes.
    Check for the correct gid value in the timing messages.
    Check for the correct parameter value when using the reference.
    """
    snoopFile = 'snoop_reference_loop3.csv'
    self.scheduleFile0 = 'reference-loop3.dot'
    if self.threadQuantity == 32:
      self.scheduleFile0 = self.scheduleFile0.replace('.dot', '-thread32.dot')
    self.downloadFile0 = self.scheduleFile0.replace('.dot', '-download.dot')
    self.snoopToCsvWithAction(snoopFile, self.actionReferenceLoop3, duration=2)
    self.startAndCheckSubprocess(('scheduleCompare', '-s', '-u', self.schedulesFolder + self.downloadFile0, self.downloadFile0), [0], 0, 0)
    self.deleteFile(self.downloadFile0)
    self.analyseFrequencyFromCsv(snoopFile, column=6, printTable=True, checkValues={'0x0002': '>3', '0x0003': '>1', '0x0004': '>1'})
    self.analyseFrequencyFromCsv(snoopFile, column=20, printTable=True, checkValues={'0x0000000000225002': '>1', '0x0000000000225003': '>3', '0x0000000000225004': '>1'})
    self.deleteFile(snoopFile)
    # setup the expected output for 8 and 32 threads.
    expectedLinesMsg1 = [
      'Msg1:',
      '  0000  00 00 00 00 00 00 00 00 10 00 09 dc 00 00 00 00  ................',
      '  0010  10 00 0a 18 00 22 50 03 10 00 0a 50 00 00 00 00  ....."P....P....',
      '  0020  00 00 00 00 00 08 61 80 a4 66 41 4b 00 00 20 02  ......a..fAK.. .',
      '  0030  10 00 09 d4',
    ]
    if self.threadQuantity == 32:
      expectedLinesMsg1[1] = '  0000  00 00 00 00 00 00 00 00 10 00 0f dc 00 00 00 00  ................'
      expectedLinesMsg1[2] = '  0010  10 00 10 18 00 22 50 03 10 00 10 50 00 00 00 00  ....."P....P....'
      expectedLinesMsg1[4] = '  0030  10 00 0f d4'
    # check the node Msg1
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, 'hex', 'Msg1'), [0], 5, 0)
    for i in range(len(lines[0])):
      self.assertEqual(expectedLinesMsg1[i], lines[0][i])

  def actionReferenceLoop3(self):
    """During snoop start pattern ref1. This produces messages with 1Hz.
    Download the schedule for later compare.
    """
    self.startPattern(self.scheduleFile0, 'ref1')
    self.delay(1.0)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', self.downloadFile0), [0], 0, 0)

  def testReferenceEdgeLoop4(self):
    """Use a schedule with a loop of a block and five tmsg and four references.
    This is not allowed. Adding the schedule fails.
    """
    self.scheduleFile0 = 'reference-loop4.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'add', self.schedulesFolder + self.scheduleFile0), [250], 2, 3)
