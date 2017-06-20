#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <boost/shared_ptr.hpp>
#include <boost/container/vector.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/optional.hpp>
#include "ftm_common.h"


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 




class Node;

typedef boost::shared_ptr<Node> node_ptr;
typedef boost::container::vector<node_ptr> npBuf;
typedef boost::container::vector<uint8_t> vBuf;
typedef boost::container::vector<uint32_t> vAdr;
typedef boost::container::vector<uint32_t> ebBuf;

extern const std::string sQM[];
extern const std::string sDL;
extern const std::string sDD;
extern const std::string sAD;
extern const std::string sTG;
extern const std::string sFD;
extern const std::string sDID;
extern const std::string sDPAR0;
extern const std::string sDPAR1;
extern const std::string sDTEF;
extern const std::string sDRES;

class  myVertex {
public:
  std::string name;
  uint32_t hash;
  node_ptr np;

  //FIXME
  //now follows a list of all possible properties graphviz_read can assign, to copy to concrete Node objects later
  //dirty business. this will have to go in the future
  // Option 1 (easy and clean, still needs ALL possible properties to be present in myVertex):
  //    pass Node constructor a reference to hosting myVertex Struct, Node can then use all relevent myVertex properties.
  //    Will make Node constructors VERY simple
  // Option 2 (hard, but very clean): 
  //    overload graphviz_read subfunctions so the parser evaluates typefield first, then have a 
  //    Node class factory and directly reference derived class members in property map.
  std::string type;

  std::string flags;

  //Meta

  //Block 
  std::string tPeriod;
  std::string rdIdxIl, rdIdxHi, rdIdxLo;
  std::string wrIdxIl, wrIdxHi, wrIdxLo;

  //Event
  std::string tOffs;

  //Timing Message
  std::string id;
  std::string id_fid;
  std::string id_gid;
  std::string id_evtno;
  std::string id_sid;
  std::string id_bpid;
  std::string id_res;

  std::string par;
  std::string tef;
  std::string res;

  //Command

  std::string tValid;


  // Flush

  std::string qIl, qHi, qLo;

  std::string frmIl, toIl;
  std::string frmHi, toHi;
  std::string frmLo, toLo; 

  //Flow, Noop
  std::string prio;
  std::string qty;

  //Wait
  std::string tWait;

  myVertex() : name("UNDEFINED"), hash(0xDEADBEEF), np(NULL), type("UNDEFINED"), flags("0xDEADBEEF"), tPeriod("0xD15EA5EDDEADBEEF"), rdIdxIl("0"), rdIdxHi("0"), rdIdxLo("0"), 
  wrIdxIl("0"), wrIdxHi("0"), wrIdxLo("0"), tOffs("0xD15EA5EDDEADBEEF"), id("0xD15EA5EDDEADBEEF"), id_fid("0"), id_gid("0"), id_evtno("0"), id_sid("0"), id_bpid("0"), id_res("0"),
  par("0xD15EA5EDDEADBEEF"), tef("0"), res("0"), tValid("0xD15EA5EDDEADBEEF"),
  qIl("0"), qHi("0"), qLo("0"), frmIl("0"), toIl("0"), frmHi("0"), toHi("0"), frmLo("0"), toLo("0"), prio("0"), qty("1"), tWait("0xD15EA5EDDEADBEEF") {}
  
  myVertex(std::string name, uint32_t hash, node_ptr np, std::string type, std::string flags) : name(name), hash(hash), np(np), type(type), flags(flags), tPeriod("0xD15EA5EDDEADBEEF"),
  rdIdxIl("0"), rdIdxHi("0"), rdIdxLo("0"), wrIdxIl("0"), wrIdxHi("0"), wrIdxLo("0"), tOffs("0xD15EA5EDDEADBEEF"), id("0xD15EA5EDDEADBEEF"), id_fid("0"), id_gid("0"), id_evtno("0"), id_sid("0"), id_bpid("0"), id_res("0"),
  par("0xD15EA5EDDEADBEEF"), tef("0"), res("0"), tValid("0xD15EA5EDDEADBEEF"), qIl("0"), qHi("0"), qLo("0"), frmIl("0"), toIl("0"), frmHi("0"), toHi("0"), frmLo("0"), toLo("0"), prio("0"), qty("1"), tWait("0xD15EA5EDDEADBEEF") {}

};




class myEdge {
public:
  std::string type;
  myEdge() : type("UNDEFINED") {}
  myEdge(std::string type) : type(type) {}
};


typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::bidirectionalS, myVertex, myEdge > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
typedef boost::container::vector<vertex_t> vVertices;



template<typename T>
inline void writeLeNumberToBeBytes(uint8_t* pB, T val) {
  T x = boost::endian::endian_reverse(val);
  std::copy(static_cast<const uint8_t*>(static_cast<const void*>(&x)),
            static_cast<const uint8_t*>(static_cast<const void*>(&x)) + sizeof x,
            pB);
}

template<typename T>
inline T writeBeBytesToLeNumber(uint8_t* pB) {
  return boost::endian::endian_reverse(*((T*)pB));
}

template<typename T>
inline T s2u(const std::string& s) {
  return (T)std::stoull(s, 0, 0);
}


inline void hexDump (const char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
  printf ("\n");  
}

inline void vHexDump (const char *desc, vBuf pc) {
    int i, len = pc.size();
    unsigned char buff[17];

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
  printf ("\n");  
}


#endif
