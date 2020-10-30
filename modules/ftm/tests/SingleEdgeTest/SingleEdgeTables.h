#ifndef _SINGLEEDGETABLES_H_
#define _SINGLEEDGETABLES_H_

#include <map>
#include <tuple>

#include "dotstr.h"

namespace dnt = DotStr::Node::TypeVal;
namespace det = DotStr::Edge::TypeVal;

/**
This namespace holds a map to return the expected test results for each test case.
The test cases are identified by the type of node1, the type of node2, and
the type of the edge from node 1 to node2.
*/
namespace SingleEdgeTest {

/**
Enumerate the test results. First approach: only distinguish exceptions and good test cases.
*/
enum TEST_RESULT { TEST_OK, TEST_EXCEPTION };
}  // namespace SingleEdgeTest

// extern std::map<std::string, SingleEdgeTest::TEST_RESULT> CategoryOrphanMap;
// extern std::map<std::pair<std::string, std::string>, SingleEdgeTest::TEST_RESULT> CategoryEdgeTypeMap;
// extern std::map<std::tuple<std::string, std::string, std::string>, SingleEdgeTest::TEST_RESULT> CategoryOutEdgeMap;
// extern std::map<std::tuple<std::string, std::string, std::string>, SingleEdgeTest::TEST_RESULT> CategoryVertexTypeMap;

/**
Map to define test OK tests.
The map maps a triple to the test result. The triple is (node1, nod2, edge from node1 to node2).
*/
extern std::map<std::tuple<std::string, std::string, std::string>, SingleEdgeTest::TEST_RESULT> CategoryTestOkMap;

/**
For a triple return the expected test result.
\param triple std::tuple (type of node1, type of node2, type of edge from node1 to node2)
*/
extern SingleEdgeTest::TEST_RESULT getExpectedResult(std::tuple<std::string, std::string, std::string> triple);

#endif
