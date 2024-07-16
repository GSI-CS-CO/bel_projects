import dm_testbench
import pathlib
import pytest

"""Start all pattern in the schedules and download status. Compare original schedule with downloaded.
Steps for a schedule:
Add schedule to datamaster
Start all patterns of this schedule
Download status of datamaster
Compare original schedule with downloaded schedule
"""
class AddDownloadCompare(dm_testbench.DmTestbench):

  def addDownloadCompareSchedule(self, scheduleFile, statusMeta=False, abortPattern=False, compareStrict=True):
    # modify the schedule file name when we have 3 CPUs or 32 threads.
    # This is needed for dynpar edges where memory addresses are stored
    # in the parameter values.
    statusFile = scheduleFile.replace('.dot', '-download.dot')
    if self.cpuQuantity == 3:
      scheduleFile3cpu = scheduleFile.replace('.dot', '-3cpu.dot')
      fileObj = pathlib.Path(self.schedulesFolder + scheduleFile3cpu)
      if fileObj.exists():
        scheduleFile = scheduleFile3cpu
    if self.threadQuantity == 32:
      scheduleFile32 = scheduleFile.replace('.dot', '-thread32.dot')
      fileObj = pathlib.Path(self.schedulesFolder + scheduleFile32)
      if fileObj.exists():
        scheduleFile = scheduleFile32
    self.startPattern(scheduleFile, 'patternA')
    if statusMeta:
      options = '-so'
      returnCode = 2
    else:
      options = '-o'
      returnCode = 0
    if compareStrict:
      # compare in silent mode.
      scheduleCompareOption = '-s'
    else:
      # compare in silent mode and interpret 'undefined' as empty string.
      scheduleCompareOption = '-us'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', options, statusFile))
    self.startAndCheckSubprocess(('scheduleCompare', scheduleCompareOption, self.schedulesFolder + scheduleFile, statusFile), [returnCode], 0, 0)
    self.deleteFile(statusFile)
    statusFile = 'statusKeep.dot'
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'keep', self.schedulesFolder + scheduleFile))
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'status', options, statusFile))
    self.startAndCheckSubprocess(('scheduleCompare', scheduleCompareOption, self.schedulesFolder + scheduleFile, statusFile), [returnCode], 0, 0)
    self.deleteFile(statusFile)
    # remove the schedule
    if abortPattern:
      self.startAndCheckSubprocess((self.binaryDmCmd, self.datamaster, 'abort'))
    self.startAndCheckSubprocess((self.binaryDmSched, self.datamaster, 'remove', self.schedulesFolder + scheduleFile))

  def generateScript(self, scheduleFile):
    """Use this to write all names of schedule into a script.
    This script can be used to move all generated schedules to the
    schedules folder.
    """
    fileName = 'moveSchedules.sh'
    with open(fileName, 'a') as file1:
      file1.write('mv dot/' + scheduleFile + ' ' + self.schedulesFolder + '\n')

  def test_aScheduleBlockBlockAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-block-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sBlock, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-block-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockBlockReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-block-reference.dot')
    # ~ {{dnt::sBlock, dnt::sBlock, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockBlockalignAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-blockalign-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sBlockAlign, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-blockalign-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockBlockalignReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-blockalign-reference.dot')
    # ~ {{dnt::sBlock, dnt::sBlockAlign, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockFlowAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-flow-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdFlow, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-flow-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockFlowReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-flow-reference.dot')
    # ~ {{dnt::sBlock, dnt::sCmdFlow, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockFlushAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-flush-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdFlush, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-flush-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockFlushReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-flush-reference.dot')
    # ~ {{dnt::sBlock, dnt::sCmdFlush, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockGlobalAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-global-altdst.dot', compareStrict=False)
    # ~ {{dnt::sBlock, dnt::sGlobal, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-global-defdst.dot', compareStrict=False)
    # ~ {{dnt::sBlock, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockGlobalReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-global-reference.dot', compareStrict=False)
    # ~ {{dnt::sBlock, dnt::sGlobal, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockNoopAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-noop-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdNoop, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-noop-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockNoopReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-noop-reference.dot')
    # ~ {{dnt::sBlock, dnt::sCmdNoop, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockOriginAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-origin-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sOrigin, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-origin-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockOriginReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-origin-reference.dot')
    # ~ {{dnt::sBlock, dnt::sOrigin, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockStartthreadAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-startthread-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sStartThread, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-startthread-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockStartthreadReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-startthread-reference.dot')
    # ~ {{dnt::sBlock, dnt::sStartThread, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockSwitchAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-switch-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sSwitch, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-switch-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockSwitchReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-switch-reference.dot')
    # ~ {{dnt::sBlock, dnt::sSwitch, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockTmsgAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-tmsg-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sTMsg, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-tmsg-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockTmsgReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-tmsg-reference.dot')
    # ~ {{dnt::sBlock, dnt::sTMsg, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockWaitAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-wait-altdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdWait, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-wait-defdst.dot')
    # ~ {{dnt::sBlock, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockWaitReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-block-wait-reference.dot')
    # ~ {{dnt::sBlock, dnt::sCmdWait, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignBlockAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-block-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sBlock, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-block-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignBlockReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-block-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sBlock, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignBlockalignAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-blockalign-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sBlockAlign, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-blockalign-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignBlockalignReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-blockalign-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sBlockAlign, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignFlowAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-flow-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlow, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-flow-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignFlowReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-flow-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlow, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignFlushAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-flush-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlush, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-flush-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignFlushReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-flush-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdFlush, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignGlobalAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-global-altdst.dot', compareStrict=False)
    # ~ {{dnt::sBlockAlign, dnt::sGlobal, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-global-defdst.dot', compareStrict=False)
    # ~ {{dnt::sBlockAlign, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignGlobalReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-global-reference.dot', compareStrict=False)
    # ~ {{dnt::sBlockAlign, dnt::sGlobal, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignNoopAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-noop-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdNoop, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-noop-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignNoopReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-noop-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdNoop, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignOriginAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-origin-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sOrigin, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-origin-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignOriginReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-origin-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sOrigin, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignStartthreadAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-startthread-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sStartThread, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-startthread-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignStartthreadReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-startthread-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sStartThread, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignSwitchAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-switch-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sSwitch, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-switch-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignSwitchReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-switch-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sSwitch, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignTmsgAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-tmsg-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sTMsg, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-tmsg-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignTmsgReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-tmsg-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sTMsg, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignWaitAltdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-wait-altdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdWait, det::sAltDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-wait-defdst.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleBlockalignWaitReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-blockalign-wait-reference.dot')
    # ~ {{dnt::sBlockAlign, dnt::sCmdWait, det::sRef}, SingleEdgeTest::TEST_OK},

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

  def test_aScheduleTmsgStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-startthread-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-origin-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgStartthreadDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-startthread-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgOriginDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-origin-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgStartthreadDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-startthread-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgOriginDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-origin-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-block-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sBlock, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgBlockalignReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-blockalign-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sBlockAlign, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlowReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flow-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlow, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgFlushReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-flush-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdFlush, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgNoopReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-noop-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdNoop, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgOriginReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-origin-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sOrigin, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgStartthreadReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-startthread-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sStartThread, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgSwitchReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-switch-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sSwitch, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgTmsgReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-tmsg-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sTMsg, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleTmsgWaitReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-wait-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sCmdWait, det::sRef}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleTmsgGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-global-defdst.dot')
    # ~ {{dnt::sTMsg, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleTmsgGlobalDynpar0(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-global-dynpar0.dot')
    # ~ {{dnt::sTMsg, dnt::sGlobal, det::sDynPar0}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleTmsgGlobalDynpar1(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-global-dynpar1.dot')
    # ~ {{dnt::sTMsg, dnt::sGlobal, det::sDynPar1}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleTmsgGlobalReference(self):
    self.addDownloadCompareSchedule('testSingleEdge-tmsg-global-reference.dot')
    # ~ {{dnt::sTMsg, dnt::sGlobal, det::sRef}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-block-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-blockalign-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-flow-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-flush-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-noop-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-switch-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-tmsg-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-wait-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopBlockTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-block-target.dot')
    # ~ {{dnt::sCmdNoop, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopBlockalignTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-blockalign-target.dot')
    # ~ {{dnt::sCmdNoop, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-startthread-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleNoopOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-noop-origin-defdst.dot')
    # ~ {{dnt::sCmdNoop, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-block-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-blockalign-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-flow-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-flush-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-noop-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-switch-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-tmsg-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-wait-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-block-target.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockalignTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-blockalign-target.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-block-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlock, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowBlockalignFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-blockalign-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sBlockAlign, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowFlowFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-flow-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlow, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowFlushFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-flush-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdFlush, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowNoopFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-noop-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdNoop, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowSwitchFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-switch-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sSwitch, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowStartthreadFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-startthread-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sStartThread, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowOriginFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-origin-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sOrigin, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowTmsgFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-tmsg-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sTMsg, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowWaitFlowdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-wait-flowdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sCmdWait, det::sCmdFlowDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-startthread-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlowOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flow-origin-defdst.dot')
    # ~ {{dnt::sCmdFlow, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-block-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-blockalign-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-flow-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-flush-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-noop-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-switch-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-tmsg-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-wait-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchBlockTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-block-target.dot')
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchBlockalignTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-blockalign-target.dot')
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sSwitchTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchBlockSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-block-switchdst.dot', abortPattern=True)
    # ~ {{dnt::sSwitch, dnt::sBlock, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchBlockalignSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-blockalign-switchdst.dot', abortPattern=True)
    # ~ {{dnt::sSwitch, dnt::sBlockAlign, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchFlowSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-flow-switchdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdFlow, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchFlushSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-flush-switchdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdFlush, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchNoopSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-noop-switchdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdNoop, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchSwitchSwitchdst(self):
    """This schedule has two switch nodes. These are switching between each
    other. Since there is no tmsg node, it is only visible that the cursor
    switches. The block C3 has a period of 1 second. Thus the pattern is
    stable during the test and we have a defined state which we can compare
    with the original schedule.
    """
    self.addDownloadCompareSchedule('testSingleEdge-switch-switch-switchdst.dot', abortPattern=True)
    # ~ {{dnt::sSwitch, dnt::sSwitch, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchTmsgSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-tmsg-switchdst.dot', abortPattern=True)
    # ~ {{dnt::sSwitch, dnt::sTMsg, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchWaitSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-wait-switchdst.dot')
    # ~ {{dnt::sSwitch, dnt::sCmdWait, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-startthread-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-origin-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchStartthreadSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-startthread-switchdst.dot', abortPattern=True)
    # ~ {{dnt::sSwitch, dnt::sStartThread, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleSwitchOriginSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-origin-switchdst.dot', abortPattern=True)
    # ~ {{dnt::sSwitch, dnt::sOrigin, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleSwitchGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-global-defdst.dot')
    # ~ {{dnt::sSwitch, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleSwitchGlobalSwitchdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-switch-global-switchdst.dot')
    # ~ {{dnt::sSwitch, dnt::sGlobal, det::sSwitchDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-block-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-blockalign-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-flow-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-flush-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-noop-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-switch-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-tmsg-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-wait-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-startthread-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-origin-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushBlockTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-block-target.dot')
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushBlockalignTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-blockalign-target.dot')
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushBlockFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-block-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sBlock, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushBlockalignFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-blockalign-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sBlockAlign, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushFlowFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-flow-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlow, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushFlushFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-flush-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdFlush, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushNoopFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-noop-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdNoop, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushSwitchFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-switch-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sSwitch, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushTmsgFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-tmsg-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sTMsg, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushWaitFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-wait-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sCmdWait, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushStartthreadFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-startthread-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sStartThread, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleFlushOriginFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-origin-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sOrigin, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleFlushGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-global-defdst.dot')
    # ~ {{dnt::sCmdFlush, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleFlushGlobalFlushovr(self):
    self.addDownloadCompareSchedule('testSingleEdge-flush-global-flushovr.dot')
    # ~ {{dnt::sCmdFlush, dnt::sGlobal, det::sCmdFlushOvr}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-block-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-blockalign-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-flow-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-flush-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-noop-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-switch-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-tmsg-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-wait-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitBlockTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-block-target.dot')
    # ~ {{dnt::sCmdWait, dnt::sBlock, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitBlockalignTarget(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-blockalign-target.dot')
    # ~ {{dnt::sCmdWait, dnt::sBlockAlign, det::sCmdTarget}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-startthread-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleWaitOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-origin-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleWaitGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-wait-global-defdst.dot')
    # ~ {{dnt::sCmdWait, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-block-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-blockalign-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-flow-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-flush-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-noop-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-origin-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-startthread-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-wait-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-switch-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleStartthreadTmsgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-tmsg-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleStartthreadGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-startthread-global-defdst.dot')
    # ~ {{dnt::sStartThread, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginBlockDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-block-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sBlock, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginBlockalignDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-blockalign-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sBlockAlign, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginFlowDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-flow-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdFlow, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginFlushDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-flush-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdFlush, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginNoopDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-noop-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdNoop, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginOriginDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-origin-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sOrigin, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginStartthreadDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-startthread-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sStartThread, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginWaitDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-wait-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdWait, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginSwitchDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-switch-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sSwitch, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginTmgDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-tmsg-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sTMsg, det::sDefDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginBlockOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-block-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sBlock, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginBlockalignOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-blockalign-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sBlockAlign, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginFlowOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-flow-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdFlow, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginFlushOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-flush-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdFlush, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginNoopOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-noop-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdNoop, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginOriginOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-origin-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sOrigin, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginStartthreadOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-startthread-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sStartThread, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginWaitOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-wait-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sCmdWait, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginSwitchOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-switch-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sSwitch, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  def test_aScheduleOriginTmsgOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-tmsg-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sTMsg, det::sOriginDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleOriginGlobalDefdst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-global-defdst.dot')
    # ~ {{dnt::sOrigin, dnt::sGlobal, det::sDefDst}, SingleEdgeTest::TEST_OK},

  @pytest.mark.development
  def test_aScheduleOriginGlobalOrigindst(self):
    self.addDownloadCompareSchedule('testSingleEdge-origin-global-origindst.dot')
    # ~ {{dnt::sOrigin, dnt::sGlobal, det::sOriginDst}, SingleEdgeTest::TEST_OK},
