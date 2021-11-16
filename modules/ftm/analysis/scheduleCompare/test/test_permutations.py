import common_scheduleCompare

"""Class tests scheduleCompare with permuted graphs.
"""
class TestPermutations(common_scheduleCompare.CommonScheduleCompare):

  def test_permutation_x0(self):
    """Structure is the same, labels of the nodes are permuted.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x0-permuted.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)

  def test_permutation_x0_n(self):
    """Structure is the same, labels of the nodes are permuted.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x0-permuted.dot', '-n', expectedReturnCode=0, linesCerr=0, linesCout=5)

  def test_permutation_test0(self):
    """The order of the nodes in the dot files is permuted. Same definition of edges.
    """
    self.callScheduleCompare('permutations/test0.dot', 'permutations/test0-permuted.dot', expectedReturnCode=0, linesCerr=0, linesCout=3)

  def test_permutation_edge_types(self):
    """The graphs have three nodes and two edges. The types of the two edges are interchanged.
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types1.dot', expectedReturnCode=1, linesCerr=0, linesCout=1)
