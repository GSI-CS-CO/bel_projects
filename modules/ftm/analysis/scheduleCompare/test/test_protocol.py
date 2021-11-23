import common_scheduleCompare

"""Class tests scheduleCompare with protocol of fails for vertices or edges.
"""
class TestProtocol(common_scheduleCompare.CommonScheduleCompare):

  def test_protocol_vertices(self):
    """Structure is the same, the vertices differ in the parameter value (one is ffff, the other is A).
    The protocol shows:
    Node: A: Result: 1, key: par, value1: '0xffff', value2: '0xA'.
    """
    self.callScheduleCompare('dot_hex/tmsg-par_ffff.dot', 'dot_hex/tmsg-par_A.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=14)

  def test_protocol_edge_types(self):
    """Structure is the same, the edge types are different.
    The protocol shows:
    """
    self.callScheduleCompare('permutations/x-edge-types0.dot', 'permutations/x-edge-types1.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=24)

  def test_protocol_case_01(self):
    """Graph 1 and graph 2 have the same size and are isomorphic.
    The protocol shows:
    Isomorphism 2, Graph 1, Vertex: a != 'b'; != 'c';
    This shows failure while constructing the second isomorphism. This is OK.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x0.dot', '-v', expectedReturnCode=0, linesCerr=0, linesCout=26)

  def test_protocol_case_02(self):
    """Graph 1 and graph 2 have the same size and are not isomorphic due to vertex comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a != 'a2'; != 'b2'; != 'c2';
    This shows failure while constructing the first isomorphism. Vertex a is not found in graph 2.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x2.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=24)

  def test_protocol_case_03(self):
    """Graph 1 and graph 2 have the same size and are not isomorphic due to vertex comparison without name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x2b.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=24)

  def test_protocol_case_04(self):
    """Graph 1 and graph 2 have the same size and are not isomorphic due to edge comparison with name comparison.
    The protocol shows: 
    Isomorphism 1, Graph 1, Vertex: a != 'b'; != 'c';
    Mapping each vertex to the vertex with the same name fails because of the different structure. Construction of
    some other isomorphism fails due to different names.
    Graph 1 is a cycle, graph 2 has an edge from a to c. Thus, the structure is different.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x0b.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=24)

  def test_protocol_case_05(self):
    """Graph 1 and graph 2 have the same size and are not isomorphic due to edge comparison without name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Edge: a -> b [xy]: Result: -1, key: type, value1: 'xy', value2: 'xy1'.Result: -1, key: type, value1: 'xy', value2: 'xy1'.Result: -1, key: type, value1: 'xy', value2: 'xy1'.
    The edge a -> b is compared to three edges of graph 2. The edge type xy is not found in graph 2.
    There is no isomorphism because graph 1 uses edge type xy and graph 2 uses edge type xy1.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x0a.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=24)

  def test_protocol_case_06(self):
    """Graph 1 has less edges than graph 2, but the same number of vertices and graph is isomorphic to a subgraph of graph 2.
    This is imposible, graph 2 has an edge, which has no source in graph 1.
    Dummy test case.
    """
    self.assertTrue(True);

  def test_protocol_case_07(self):
    """Graph 1 has less edges than graph 2, but the same number of vertices and are not isomorphic due to vertex comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a1 != 'a'; != 'b'; != 'c';
    Vertex a1 is not found in graph 2.
    """
    self.callScheduleCompare('permutations/x0d.dot', 'permutations/x0.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=23)

  def test_protocol_case_08(self):
    """Graph 1 has less edges than graph 2, but the same number of vertices and are not isomorphic due to vertex comparison without name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a1 compare: 1, key: par, value1: '1', value2: ''. compare: 1, key: par, value1: '1', value2: ''. compare: 1, key: par, value1: '1', value2: ''.
    Vertices of graph 1 have a parameter, value 1 but vertices of graph 2 have no parameter. This causes vertex comparison to fail.
    """
    self.callScheduleCompare('permutations/x0e.dot', 'permutations/x0.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=23)

  def test_protocol_case_09(self):
    """Graph 1 has less edges than graph 2, but the same number of vertices and are not isomorphic due to edge comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a != 'b'; != 'c';
    There is no edge from c to a in graph 1.
    """
    self.callScheduleCompare('permutations/x0c.dot', 'permutations/x0.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=23)

  def test_protocol_case_10(self):
    """Graph 1 has less edges than graph 2, but the same number of vertices and are not isomorphic due to edge comparison without name comparison.
    The protocol shows: nothing.
    There is no edge from c to a in graph 1. since vertex names are not compared, no protocol is shown.
    """
    self.callScheduleCompare('permutations/x0c.dot', 'permutations/x0.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=22)

  def test_protocol_case_11(self):
    """Graph 1 has less vertices than graph 2, but the same number of edges and graph 1 is isomorphic to a subgraph of graph 2.
    This is imposible. Each subset of vertices of graph 2 which has as many vertices than graph 1 has less edges than graph 1.
    Thus there is no isomorphism.
    Dummy test case.
    """
    self.assertTrue(True);

  def test_protocol_case_12(self):
    """Graph 1 has less vertices than graph 2, but the same number of edges and are not isomorphic due to vertex comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a != 'a1'; != 'b1'; != 'c1'; != 'd1';
    Vertex a of graph 1 is not found in graph 2.
    Graph 1 is a cycle with 3 vertices and graph 2 is a path with 4 vertices.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x4.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=25)

  def test_protocol_case_13(self):
    """Graph 1 has less vertices than graph 2, but the same number of edges and are not isomorphic due to vertex comparison without name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'.
    Graph 1 is a cycle with 3 vertices and graph 2 is a path with 4 vertices.
    Vertices in graph 2 have a parameter value 1. Vertices in graph 1 have no parameter.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x4a.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=25)

  def test_protocol_case_14(self):
    """Graph 1 has less vertices than graph 2, but the same number of edges and are not isomorphic due to edge comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a != 'b'; != 'c'; != 'd';
    Graph 1 is a cycle with 3 vertices and graph 2 is a path with 4 vertices.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x4b.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=25)

  def test_protocol_case_15(self):
    """Graph 1 has less vertices than graph 2, but the same number of edges and are not isomorphic due to edge comparison without name comparison.
    The protocol shows: nothing (protocol implementation to be improved).
    Graph 1 is a cycle with 3 vertices and graph 2 is a path with 4 vertices.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x4.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=24)

  def test_protocol_case_16(self):
    """Graph 1 has less vertices than graph 2 and less edges and graph 1 is isomorphic to a subgraph of graph 2.
    The protocol shows:
    Isomorphism 2, Graph 1, Vertex: a != 'b'; != 'c'; != 'd'; != 'e';
    Isomorphism 2, Graph 1, Vertex: c != 'd';
    This protocol shows that the construction of the second isomorphism fails.
    There is one isomorphism (0, 0) (1, 1) (2, 2). 
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1.dot', '-v', expectedReturnCode=2, linesCerr=0, linesCout=33)

  def test_protocol_case_17(self):
    """Graph 1 has less vertices than graph 2 and less edges and are not isomorphic due to vertex comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a != 'a1'; != 'b1'; != 'c1'; != 'd1'; != 'e1';
    Vertex a is not found in graph 2.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1a.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=30)

  def test_protocol_case_18(self):
    """Graph 1 has less vertices than graph 2 and less edges and are not isomorphic due to vertex comparison without name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'. compare: -1, key: par, value1: '', value2: '1'.
    Vertices in graph 2 have a parameter value 1. Vertices in graph 1 have no parameter.
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x1b.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=30)

  def test_protocol_case_19(self):
    """Graph 1 has less vertices than graph 2 and less edges and are not isomorphic due to edge comparison with name comparison.
    The protocol shows:
    Isomorphism 1, Graph 1, Vertex: a != 'b'; != 'c'; != 'd'; != 'e';
    Graph 1 is a cycle with 3 vertices and graph 2 is a path with 5 vertices. 
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x5.dot', '-v', expectedReturnCode=1, linesCerr=0, linesCout=27)

  def test_protocol_case_20(self):
    """Graph 1 has less vertices than graph 2 and less edges and are not isomorphic due to edge comparison without name comparison.
    The protocol shows: nothing (protocol implementation to be improved).
    Graph 1 is a cycle with 3 vertices and graph 2 is a path with 5 vertices. 
    """
    self.callScheduleCompare('permutations/x0.dot', 'permutations/x5.dot', '-vn', expectedReturnCode=1, linesCerr=0, linesCout=26)
