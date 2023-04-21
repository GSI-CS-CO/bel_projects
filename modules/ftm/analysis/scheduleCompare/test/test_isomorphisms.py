import common_scheduleCompare

"""Class tests scheduleCompare without comparing names of vertices.
This mode finds all isomorphisms between two schedules.
"""
class TestIsomorphisms(common_scheduleCompare.CommonScheduleCompare):

  def test_six_isomorphisms(self):
    """Compare a cycle of length 3 with a schedule of 4 vertices and 5 edges.
    There are 6 isomorphisms.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/2cycles-a1b1c1d1.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)

  def test_six_isomorphisms_n(self):
    """Compare a cycle of length 3 with a schedule of 4 vertices and 5 edges.
    There are 6 isomorphisms.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/2cycles-a1b1c1d1.dot', '-n', expectedReturnCode=2, linesCerr=0, linesCout=8)

  def test_six_isomorphisms_silent(self):
    """Compare a cycle of length 3 with a schedule of 4 vertices and 5 edges.
    There is no isomorphism with name comparison (default). Test with silent (-s) option.
    The vertex names of the first graph are not found in the second graph.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/2cycles-a1b1c1d1.dot', '-s', expectedReturnCode=1, linesCerr=0, linesCout=0)

  def test_six_isomorphisms_silent_n(self):
    """Compare a cycle of length 3 with a schedule of 4 vertices and 5 edges.
    There are 6 isomorphisms. Test with silent (-s) option.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/2cycles-a1b1c1d1.dot', '-sn', expectedReturnCode=2, linesCerr=0, linesCout=0)

  def test_three_isomorphisms(self):
    """Compare a cycle of length 3 with itself.
    There is 1 isomorphism with name comparison (default).
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3-cycle-abc.dot', expectedReturnCode=0, linesCerr=0, linesCout=3)

  def test_three_isomorphisms_n(self):
    """Compare a cycle of length 3 with itself.
    There are 3 isomorphisms.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3-cycle-abc.dot', '-n', expectedReturnCode=0, linesCerr=0, linesCout=5)

  def test_nine_isomorphisms(self):
    """Compare a cycle of length 3 with a schedule ...
    There is one isomorphism with name comparison (default).
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3cycles-abcde.dot', expectedReturnCode=2, linesCerr=0, linesCout=3)

  def test_nine_isomorphisms_n(self):
    """Compare a cycle of length 3 with a schedule ...
    There are 9 isomorphisms.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3cycles-abcde.dot', '-n', expectedReturnCode=2, linesCerr=0, linesCout=11)

  def test_three_isomorphisms_different_names(self):
    """Compare a cycle of length 3 with another cycle of length 3 (different vertex names).
    There is no isomorphism with name comparison (default).
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3-cycle-a1b1c1.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)

  def test_three_isomorphisms_different_names_n(self):
    """Compare a cycle of length 3 with another cycle of length 3.
    There are 3 isomorphisms.
    """
    self.callScheduleCompare('permutations/3-cycle-abc.dot', 'permutations/3-cycle-a1b1c1.dot', '-n', expectedReturnCode=0, linesCerr=0, linesCout=5)

  def test_edge_types_isomorphisms(self):
    """Compare two schedules with three nodes and two edges.
    The edge types are switched. No isomorphism with name comparison (default).
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types1.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)

  def test_edge_types_isomorphisms_n(self):
    """Compare two schedules with three nodes and two edges.
    The edge types are switched. One isomorphism.
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types1.dot', '-n', expectedReturnCode=0, linesCerr=0, linesCout=3)

  def test_edge_types_isomorphisms_invalid(self):
    """Compare two schedules with three nodes and two edges.
    The one edge type is invalid. Schedules are not isomorphic.
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types0a.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)

  def test_edge_types_isomorphisms_invalid_n(self):
    """Compare two schedules with three nodes and two edges.
    The one edge type is invalid. Schedules are not isomorphic.
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types0a.dot', '-n', expectedReturnCode=1, linesCerr=0, linesCout=1)

