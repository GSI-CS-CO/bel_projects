#include <etherbone.h>
#include <tlu.h>
#include <eca.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h> // sleep

using namespace GSI_ECA;
using namespace GSI_TLU;

#define HIGH_NS		100000
#define LOW_NS		100000
#define CPU_DELAY_MS	20
#define EVENTS		16
#define MAX_CHANNELS	32
#define TAG_BIT		0x40000000

int main(int argc, const char** argv) {
  Socket socket;
  Device device;
  status_t status;

  if (argc != 2) {
    fprintf(stderr, "%s: expecting argument <device>\n", argv[0]);
    return 1;
  }
  
  socket.open();
  if ((status = device.open(socket, argv[1])) != EB_OK) {
    fprintf(stderr, "%s: failed to open %s: %s\n", argv[0], argv[1], eb_status(status));
    return 1;
  }
  
  /* Find the ECA */
  std::vector<ECA> ecas;
  ECA::probe(device, ecas);
  assert (ecas.size() == 1);
  ECA& eca = ecas[0];
  
  /* Find the TLU */
  std::vector<TLU> tlus;
  TLU::probe(device, tlus);
  assert (tlus.size() == 1);
  TLU& tlu = tlus[0];
  
  /* Configure the TLU to record rising edge timestamps */
  tlu.hook(-1, false);
  tlu.set_enable(false); // no interrupts, please
  tlu.clear(-1);
  tlu.listen(-1, true, true, 8); /* Listen on all inputs */
  
  while (1) { 
    std::vector<std::vector<uint64_t> > queues;
    tlu.pop_all(queues);
    for (unsigned i = 0; i < queues.size(); ++i) {
      for (unsigned j = 0; j < queues[i].size(); ++j) {
        printf("Input %d saw an event! %s\n", i, eca.date(queues[i][j]/8).c_str());
      }
    }
    usleep(20000); // 50Hz is enough
  }
  
  return 0;
}
