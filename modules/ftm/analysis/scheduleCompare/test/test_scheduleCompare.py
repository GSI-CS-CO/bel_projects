import common_scheduleCompare

"""
Class collects unit tests for scheduleCompare.
Tests run scheduleCompare with two dot files and check the result.
In addition, run scheduleCompare in test mode, comparing a dot file with itself.
"""
class TestScheduleCompare(common_scheduleCompare.CommonScheduleCompare):

  def test_first_isomorphism(self):
    self.callScheduleCompare('permutations/test0.dot', 'permutations/test0.dot', expectedReturnCode=0, linesCerr=0, linesCout=3)

  def test_first_isomorphism_verbose(self):
    self.callScheduleCompare('permutations/test0.dot', 'permutations/test0.dot', '-v', expectedReturnCode=0, linesCerr=0, linesCout=26)

  def test_usage_message(self):
    self.callScheduleCompare('', '', '-h', expectedReturnCode=14, linesCerr=26, linesCout=0)

  def test_folder_dot_tmsg(self):
    self.allPairsFilesInFolderTest('dot_tmsg/')

  def test_folder_dot_block(self):
    self.allPairsFilesInFolderTest('dot_block/')

  def test_folder_dot_flow(self):
    self.allPairsFilesInFolderTest('dot_flow/')

  def test_folder_dot_flush(self):
    self.allPairsFilesInFolderTest('dot_flush/')

  def test_folder_dot_switch(self):
    self.allPairsFilesInFolderTest('dot_switch/')

  def test_folder_dot_wait(self):
    self.allPairsFilesInFolderTest('dot_wait/')

  def test_folder_dot_hex(self):
    self.callScheduleCompare('dot_hex/tmsg-par_A.dot', 'dot_hex/tmsg-par_A.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_A.dot', 'dot_hex/tmsg-par_10.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_10.dot', 'dot_hex/tmsg-par_A.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_10.dot', 'dot_hex/tmsg-par_10.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_A.dot', 'dot_hex/tmsg-par_000A.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_000a.dot', 'dot_hex/tmsg-par_000A.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_ffff.dot', 'dot_hex/tmsg-par_A.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_ffff.dot', 'dot_hex/tmsg-par_000A.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_ffff.dot', 'dot_hex/tmsg-par_000a.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_FFFFEEEEDDDDCCCC.dot', 'dot_hex/tmsg-par_A.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_FFFFEEEEDDDDCCCC.dot', 'dot_hex/tmsg-par_000a.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_FFFFEEEEDDDDCCCC.dot', 'dot_hex/tmsg-par_000A.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_FFFFEEEEDDDDCCCC.dot', 'dot_hex/tmsg-par_ffff.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_hex/tmsg-par_FFFFEEEEDDDDCCCC.dot', 'dot_hex/tmsg-par_FFFFEEEEDDDDCCCC.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)

  def test_folder_dot_graph_entries(self):
    self.allPairsFilesInFolderTest('dot_graph_entries/')

  def test_dot_graph_entries_2(self):
    self.callScheduleCompare('dot_graph_entries/graph-entry-008600.dot', 'dot_graph_entries_2/graph-entry-009852.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries/graph-entry-008600.dot', 'dot_graph_entries_2/graph-entry-008541.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries/graph-entry-010773.dot', 'dot_graph_entries_2/graph-entry-010745.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries/graph-entry-016159.dot', 'dot_graph_entries_2/graph-entry-016193.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_graph_entries_2/pro_2020_11_24.dot', 'dot_graph_entries_2/pro_2020_11_24.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.allFilesInFolderTest('dot_graph_entries_2/')

  def test_dot_boolean(self):
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_0.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_False.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_false.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_1.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_True.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_true.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_0.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_true.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/wait-target-tvalid-tabs_x.dot', 'dot_boolean/wait-target-tvalid-tabs_x.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_false.dot', 'dot_boolean/tmsg-target-tvalid-patentry_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_false.dot', 'dot_boolean/tmsg-target-tvalid-patentry_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_false.dot', 'dot_boolean/tmsg-target-tvalid-patentry_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_False.dot', 'dot_boolean/tmsg-target-tvalid-patentry_false.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_False.dot', 'dot_boolean/tmsg-target-tvalid-patentry_False.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_False.dot', 'dot_boolean/tmsg-target-tvalid-patentry_True.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_True.dot', 'dot_boolean/tmsg-target-tvalid-patentry_false.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_True.dot', 'dot_boolean/tmsg-target-tvalid-patentry_False.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)
    self.callScheduleCompare('dot_boolean/tmsg-target-tvalid-patentry_True.dot', 'dot_boolean/tmsg-target-tvalid-patentry_True.dot', '-s', expectedReturnCode=0, linesCerr=0, linesCout=0)

  def test_folder_dot(self):
    self.allPairsFilesInFolderTest('dot1/')
