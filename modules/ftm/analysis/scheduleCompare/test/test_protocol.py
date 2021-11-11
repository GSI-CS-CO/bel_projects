import common_scheduleCompare

"""Class tests scheduleCompare with protocol of fails for vertices or edges.
"""
class TestProtocol(common_scheduleCompare.CommonScheduleCompare):

  def test_protocol_vertices(self):
    """Structure is the same, the vertices differ in the paramter value (one is ffff, the other is A).
    The protocol shows:
    Node: A: Result: 1, key: par, value1: '0xffff', value2: '0xA'.
    """
    self.callScheduleCompare('dot_hex/tmsg-par_ffff.dot', 'dot_hex/tmsg-par_A.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=15)

  def test_protocol_edge_types(self):
    """Structure is the same, the edge types are different.
    The protocol shows:
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types1.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=23)
