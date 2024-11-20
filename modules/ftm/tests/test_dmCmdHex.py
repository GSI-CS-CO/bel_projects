import dm_testbench    # contains super class

"""
Module collects tests for dm-cmd with the command 'hex < node>'.
"""
class HexTests(dm_testbench.DmTestbench):

  def testHexdumpTarget(self):
    """Prepare all threads on all CPUs.
    Compare the output of 'dm-cmd hex Block0a' and
    'dm-cmd hex Block0a_ListDst_0' with the expected lines.
    """
    # setup the expected output for 8 and 32 threads.
    expectedLinesBlock0a=[
      "Block0a:",
      "  0000  00 00 00 00 3b 9a ca 00 00 00 00 00 10 00 0c ac  ....;...........",
      "  0010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................",
      "  0020  00 00 00 00 00 00 00 00 22 40 8c 2f 00 10 81 07  ........\"@./....",
      "  0030  10 00 0b 0c"
    ]
    if self.threadQuantity == 32:
      expectedLinesBlock0a[1] = "  0000  00 00 00 00 3b 9a ca 00 00 00 00 00 10 00 12 ac  ....;..........."
      expectedLinesBlock0a[4] = "  0030  10 00 11 0c"
    expectedLinesBlock0a_ListDst_0=[
      "Block0a_ListDst_0:",
      "  0000  10 00 0b 0c 00 00 00 00 00 00 00 00 00 00 00 00  ................",
      "  0010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................",
      "  0020  00 00 00 00 00 00 00 00 06 78 81 46 00 00 00 0c  .........x.F....",
      "  0030  10 00 09 6c"
    ]
    if self.threadQuantity == 32:
      expectedLinesBlock0a_ListDst_0[1] = "  0000  10 00 11 0c 00 00 00 00 00 00 00 00 00 00 00 00  ................"
      expectedLinesBlock0a_ListDst_0[4] = "  0030  10 00 0f 6c"
    # run schedules on CPU 0.
    self.prepareRunThreads(1)
    # the command to test: dm-cmd hex, applied for two nodes.
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, 'hex', 'Block0a'), [0], 5, 0)
    for i in range(len(lines[0])):
      self.assertEqual(expectedLinesBlock0a[i], lines[0][i])
    lines = self.startAndGetSubprocessOutput((self.binaryDmCmd, self.datamaster, 'hex', 'Block0a_ListDst_0'), [0], 5, 0)
    for i in range(len(lines[0])):
      self.assertEqual(expectedLinesBlock0a_ListDst_0[i], lines[0][i])

