import common_scheduleCompare

"""Class tests scheduleCompare without comparing names of vertices.
This mode finds all isomorphisms between two schedules.
"""
class TestIsomorphisms(common_scheduleCompare.CommonScheduleCompare):

  def test_subgraph_isomorphism(self):
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1.dot', expectedReturnCode=2, linesCerr=0, linesCout=3)

  def test_subgraph_isomorphism_verbose(self):
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1.dot', '-v', expectedReturnCode=2, linesCerr=0, linesCout=33)

  def test_subgraph_isomorphism_superverbose(self):
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1.dot', '-vv', expectedReturnCode=2, linesCerr=0, linesCout=53)

  def test_subgraph_isomorphism_silent(self):
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1.dot', '-s', expectedReturnCode=2, linesCerr=0, linesCout=0)
