import dm_testbench
import pytest

"""Start all pattern in the schedules and download status. Compare original schedule with downloaded.
Steps for a schedule:
Add schedule to datamaster
Start all patterns of this schedule
Download status of datamaster
Compare original schedule with downloaded schedule
"""
class AddDownloadCompare(dm_testbench.DmTestbench):
  def addDownloadCompareSchedule(self, scheduleFile):
    status_file = 'status.dot'
    self.startAllPattern(scheduleFile)
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', '-o', status_file))
    self.startAndCheckSubprocess(('scheduleCompare', self.schedules_folder + scheduleFile, status_file))
    self.deleteFile(status_file)

  def test_aScheduleTmsgBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-block-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-blockalign-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flow-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flush-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-noop-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-switch-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-tmsg-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-wait-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-block-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sBlock, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockalignDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-blockalign-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sBlockAlign, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlowDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flow-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlow, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlushDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flush-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlush, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgNoopDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-noop-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdNoop, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgSwitchDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-switch-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sSwitch, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgTmsgDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-tmsg-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sTMsg, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgWaitDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-wait-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdWait, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-block-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sBlock, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockalignDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-blockalign-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sBlockAlign, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlowDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flow-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlow, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlushDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flush-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlush, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgNoopDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-noop-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdNoop, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgSwitchDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-switch-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sSwitch, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgTmsgDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-tmsg-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sTMsg, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgWaitDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-wait-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdWait, det::sDynPar1}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-block-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-tmsg-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-block-target.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleFlowBlockFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-block-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlow, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlush, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdNoop, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sSwitch, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sTMsg, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sCmdWait, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-tmsg-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdFlow, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdFlush, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdNoop, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sSwitch, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sTMsg, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sCmdWait, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-tmsg-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlow, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlush, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdNoop, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sSwitch, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sTMsg, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sCmdWait, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sBlock, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockBlockalignAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-blockalign-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sBlockAlign, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdFlow, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdFlush, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdNoop, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sSwitch, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sTMsg, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sCmdWait, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sDstList, det::sDstList}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sQInfo, det::sQPrio[0]}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sQInfo, det::sQPrio[1]}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sQInfo, det::sQPrio[2]}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sBlock, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sBlockAlign, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlow, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlush, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdNoop, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sSwitch, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sTMsg, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sCmdWait, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sDstList, det::sDstList}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sQInfo, det::sQPrio[0]}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sQInfo, det::sQPrio[1]}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sQInfo, det::sQPrio[2]}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sQInfo, dnt::sQBuf, det::sMeta}, SingleEdgeTest::TEST_OK},

    # ~ {{dnt::sCmdWait, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdWait, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sDynPar0}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sDynPar0}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sDynPar1}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sDynPar1}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sStartThread, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sSwitch, dnt::sOrigin, det::sSwitchDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sStartThread, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlow, dnt::sOrigin, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sStartThread, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdFlush, dnt::sOrigin, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sCmdNoop, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sStartThread, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlock, dnt::sOrigin, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sStartThread, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sBlockAlign, dnt::sOrigin, det::sAltDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sStartThread, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sBlock, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sBlockAlign, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdFlow, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdFlush, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdNoop, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sOrigin, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sStartThread, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sCmdWait, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sSwitch, det::sOriginDst}, SingleEdgeTest::TEST_OK},
    # ~ {{dnt::sOrigin, dnt::sTMsg, det::sOriginDst}, SingleEdgeTest::TEST_OK},
