import dm_testbench
import pytest

"""
Test cases for edges of type reference.
These edges have the additional attributes
fieldtail: offset into the source node, fieldhead: offset into 
the target node, fieldwidth: width of the referenced field.
"""
class ReferenceEdge(dm_testbench.DmTestbench):
  
  @pytest.mark.development
  def testReferenceEdgeSimple(self):
    """Use a schedule with an edge of type reference between two loops
    (a block and a tmsg). The loops run with 10Hz.
    Check for the correct parameter value when using the reference.
    """
    snoopFile = 'snoop_reference1.csv'
    self.scheduleFile0 = 'reference1.dot'
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
