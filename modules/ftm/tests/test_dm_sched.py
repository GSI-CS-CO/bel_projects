import dm_testbench

"""Class collects unit tests for the command line of dm-sched.
"""
class TestDmSched(dm_testbench.DmTestbench):

  def test_sched_usage(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, '-h'],
         expectedReturnCode=[0], linesCout=0, linesCerr=25)

  def test_sched_default(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster],
         expectedReturnCode=[0], linesCout=8, linesCerr=0)

  def test_sched_status(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'status'],
         expectedReturnCode=[0], linesCout=8, linesCerr=0)

  def test_sched_dump(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'dump'],
         expectedReturnCode=[0], linesCout=0, linesCerr=0)
    linesOut = self.startAndGetSubprocessOutput(['wc', 'download.dot'],
         expectedReturnCode=[0], linesCout=1, linesCerr=0)[0]
    self.assertEqual(linesOut[0], "  4  30 191 download.dot")
    self.deleteFile('download.dot')

  def test_sched_dump_o(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'dump', '-o', 'dump.dot'],
         expectedReturnCode=[0], linesCout=0, linesCerr=0)
    linesOut = self.startAndGetSubprocessOutput(['wc', 'dump.dot'],
         expectedReturnCode=[0], linesCout=1, linesCerr=0)[0]
    self.assertEqual(linesOut[0], "  4  30 191 dump.dot")
    self.deleteFile('dump.dot')

  def test_sched_clear(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'clear'],
         expectedReturnCode=[0], linesCout=0, linesCerr=0)

  def test_sched_rawvisited(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'rawvisited'],
         expectedReturnCode=[0], linesCout=0, linesCerr=0)

  def test_sched_add_pps(self):
    self.startAndGetSubprocessOutput([self.binaryDmSched, self.datamaster, 'add', self.schedules_folder + 'pps.dot' ],
         expectedReturnCode=[0], linesCout=0, linesCerr=0)
