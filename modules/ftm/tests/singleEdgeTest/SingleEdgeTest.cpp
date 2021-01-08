#include "SingleEdgeTest.h"

#include <algorithm>  // for std::for_each
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/utility.hpp>  // for boost::tie
#include <iostream>           // for std::cout
#include <map>
#include <utility>

#include "SingleEdgeGraph.h"
#include "SingleEdgeStatus.h"
#include "SingleEdgeTables.h"
#include "block.h"
#include "carpeDM.h"
#include "event.h"
#include "graph.h"
#include "meta.h"

namespace det = DotStr::Edge::TypeVal;

std::list<std::string> edgeTypes = {
    det::sDstList,      det::sQPrio[PRIO_LO], det::sQPrio[PRIO_HI], det::sQPrio[PRIO_IL], det::sDynId,     det::sDynPar0, det::sDynPar1,
    det::sSwitchTarget,  // Links to Switch's Target
    det::sCmdTarget,    det::sCmdFlowDst,
    det::sSwitchDst,  // Links to Switch's Target Block
    det::sCmdFlushOvr,  det::sMeta,           det::sAltDst,         det::sDefDst,         det::sBadDefDst,
};

int main(int, char *[]) {
  Validation::init();
  CarpeDM cdm;
  SingleEdgeStatus status;
  int counterV1 = 0;  // counts the types of vertex v1.
  std::pair<std::string, int> entry1;
  std::pair<std::string, int> entry2;
  BOOST_FOREACH (entry1, nodeMap) {
    std::string nodeT1 = entry1.first;
    int counterE = 0;  // counts the types of edges. Reset with outer loop.
    BOOST_FOREACH (std::string edgeT, edgeTypes) {
      int counterV2 = 0;  // counts the types of vertex v2. Reset with two outer loops
      BOOST_FOREACH (entry2, nodeMap) {
        std::string nodeT2 = entry2.first;
        bool checkedException = false;
        bool knownException = false;
        SingleEdgeGraph singleEdgeGraph = SingleEdgeGraph(&cdm, nodeT1, nodeT2, edgeT);
        Graph g = singleEdgeGraph.getGraph();
        try {
          BOOST_FOREACH (vertex_t v, vertices(g)) { Validation::neighbourhoodCheck(v, g); }
          status.increment("Test ok");
        } catch (std::runtime_error e) {
          if (getExpectedResult(make_tuple(nodeT1, nodeT2, edgeT)) != SingleEdgeTest::TEST_EXCEPTION) {
            std::cout << std::setfill(' ') << std::setw(4) << status.get("All cases") << ", " << std::setw(4) << status.get("Exceptions") << ", " << std::setw(4)
                      << status.get("Known exceptions") << ": " << std::setw(10) << nodeT1 << ", " << std::setw(10) << nodeT2 << ", " << std::setw(10) << edgeT << ", ("
                      << std::setw(2) << counterV1 << "," << std::setw(2) << counterV2 << "," << std::setw(2) << counterE << ") "
                      << getExpectedResult(make_tuple(nodeT1, nodeT2, edgeT)) << " " << e.what();
          }
          checkedException = true;
          status.increment("Exceptions");
          // check for known exceptions
          if (std::string(e.what()).find("Node 'A1' of type '" + nodeT1 + "' must not have edge of type") != std::string::npos) {
            status.increment("Known exceptions");
            status.increment("Forbidden edge type");
            knownException = true;
          }
          if (std::string(e.what()).find("Node 'A1' of type '" + nodeT1 + "' with edge of type '" + edgeT + " must not have children of type") != std::string::npos) {
            status.increment("Known exceptions");
            status.increment("Forbidden child type");
            knownException = true;
          }
          if (std::string(e.what()).find("Node 'A1' of type '" + nodeT1 + "' cannot be an orphan") != std::string::npos) {
            status.increment("Known exceptions");
            status.increment("Orphan node");
            knownException = true;
          }
          if (std::string(e.what()).find("Node 'B2' of type '" + nodeT2 + "' cannot be childless") != std::string::npos) {
            status.increment("Known exceptions");
            status.increment("Childless node");
            knownException = true;
          }
          if (std::string(e.what()).find("Node 'A1' of type '" + nodeT1 + "' was found unallocated") != std::string::npos) {  // this case is obsolete.
            status.increment("Known exceptions");
            status.increment("Unallocated node");
            knownException = true;
          }
          if (!knownException) {
            std::cout << std::setfill(' ') << std::setw(4) << status.get("All cases") << ", " << std::setw(4) << status.get("Exceptions") << ", " << std::setw(4)
                      << status.get("Known exceptions") << ": " << std::setw(10) << nodeT1 << ", " << std::setw(10) << nodeT2 << ", " << std::setw(10) << edgeT << ", ("
                      << std::setw(2) << counterV1 << "," << std::setw(2) << counterV2 << "," << std::setw(2) << counterE << ") "
                      << getExpectedResult(make_tuple(nodeT1, nodeT2, edgeT)) << " " << e.what();
          }
        }
        if (!checkedException && getExpectedResult(make_tuple(nodeT1, nodeT2, edgeT)) == SingleEdgeTest::TEST_EXCEPTION) {
          std::cout << std::setfill(' ') << std::setw(4) << status.get("All cases") << ", " << std::setw(4) << status.get("Exceptions") << ", " << std::setw(4)
                    << status.get("Known exceptions") << ": " << std::setw(10) << nodeT1 << ", " << std::setw(10) << nodeT2 << ", " << std::setw(10) << edgeT << ", ("
                    << std::setw(2) << counterV1 << "," << std::setw(2) << counterV2 << "," << std::setw(2) << counterE << ") "
                    << getExpectedResult(make_tuple(nodeT1, nodeT2, edgeT)) << std::endl;
        }
        status.increment("All cases");
        try {
          singleEdgeGraph.writeDotFile(nodeT1 + "-" + nodeT2 + "-" + edgeT);
        } catch (std::runtime_error e) {
          std::cout << std::setfill(' ') << std::setw(4) << status.get("All cases") << ", " << std::setw(4) << status.get("Exceptions") << ", " << std::setw(4)
                    << status.get("Known exceptions") << ": " << std::setw(10) << nodeT1 << ", " << std::setw(10) << nodeT2 << ", " << std::setw(10) << edgeT << ", ("
                    << std::setw(2) << counterV1 << "," << std::setw(2) << counterV2 << "," << std::setw(2) << counterE << ") "
                    << getExpectedResult(make_tuple(nodeT1, nodeT2, edgeT)) << " " << e.what() << std::endl;
        }
        counterV2++;
      }
      counterE++;
    }
    counterV1++;
  }
  std::cout << "Test Status:" << std::endl;
  status.printStatus();
  return 0;
}
