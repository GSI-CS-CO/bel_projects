#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "common.h"
#include "event.h"
#include "timeblock.h"
#include <algorithm>


static void hexDump (char *desc, void *addr, int len) {
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
}

static void hexDump (char *desc, itBuf ib, int off, int len) {
    int i;
    unsigned char buff[17];


    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = off; i < off + len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", ib[i]);

        // And store a printable ASCII character for later.
        if ((ib[i] < 0x20) || (ib[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = ib[i];
        buff[(i % 16) + 1] = '\0';
    }
}



const bool MyDataSortPredicate(const boost::shared_ptr<Event> e1, const boost::shared_ptr<Event> e2)
  {
    return e1->tOffs < e2->tOffs;
  }







int main() {

  


  boost::container::vector<evt_ptr> test;

  boost::container::vector<evt_ptr>::iterator it;

  test.push_back((evt_ptr) new TimingMsg(0xcafebabedeadbee7ULL, 0, 1, 2, 4));
  test.push_back((evt_ptr) new TimingMsg(0xcafebabedeadbee6ULL, 0, 1, 2, 4));
  test.push_back((evt_ptr) new      Flow(0xcafebabedeadbee8ULL, 6, 7, 8, NULL));





 
  for(it=test.begin() ; it < test.end(); it++) {
    (*it)->show(it - test.begin(), "  ");
  } 

  std::sort(test.begin(), test.end(), MyDataSortPredicate);

  printf ("\n\n ----------------------- Sorted by Time: \n\n");

  for(it=test.begin() ; it < test.end(); it++) {
    (*it)->show(it - test.begin(), "  ");
  } 

  
  vBuf myVec(8192);
  itBuf ib = myVec.begin();

  int off = 0;

  for(it=test.begin() ; it < test.end(); it++) {
    (*it)->serialise(ib + off);
    
    printf ("\n ");
    hexDump ("Vector Dump\n", ib, off, _EVT_SIZE);
    printf ("\n");
      
    off += _EVT_SIZE;
  } 



  return 0;	
}





