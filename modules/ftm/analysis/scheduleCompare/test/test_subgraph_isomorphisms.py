import common_scheduleCompare

"""Class tests scheduleCompare without comparing names of vertices.
This mode finds all isomorphisms between two schedules.
"""
class TestIsomorphisms(common_scheduleCompare.CommonScheduleCompare):

  def test_subgraph_isomorphism(self):
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3cycles-abcde.dot', expectedReturnCode=2, linesCerr=0, linesCout=3)

  def test_subgraph_isomorphism_verbose(self):
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3cycles-abcde.dot', '-v', expectedReturnCode=2, linesCerr=0, linesCout=33)

  def test_subgraph_isomorphism_superverbose(self):
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3cycles-abcde.dot', '-vv', expectedReturnCode=2, linesCerr=0, linesCout=53)

  def test_subgraph_isomorphism_silent(self):
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3cycles-abcde.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
