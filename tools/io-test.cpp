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
  
  /* Configure ECA to create IO pulses on GPIO and LVDS */
  eca.channels[0].drain(false); // GPIO
  eca.channels[1].drain(true);  // PCIe
  eca.channels[2].drain(false); // LVDS
  eca.channels[0].freeze(false);
  eca.channels[1].freeze(false);
  eca.channels[2].freeze(false);
  eca.disable(true);
  eca.interrupt(false);
  
  Table table;
  for (int i = 0; i < EVENTS; ++i) {
    table.add(TableEntry(0xdeadbeef, i*(HIGH_NS+LOW_NS)/8,             0x0000ffffU,             0, 64));
    table.add(TableEntry(0xdeadbeef, i*(HIGH_NS+LOW_NS)/8 + HIGH_NS/8, 0xffff0000U,             0, 64));
    table.add(TableEntry(0xdeadbeef, i*(HIGH_NS+LOW_NS)/8,             0x00000FFFU + (i << 29), 2, 64));
    table.add(TableEntry(0xdeadbeef, i*(HIGH_NS+LOW_NS)/8 + HIGH_NS/8, 0x00FFF000U + (i << 29), 2, 64));
  }
  eca.store(table);
  eca.flipTables();
  
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
  
  eca.disable(false);
  
  /* Find the IO reconfig */
  std::vector<sdb_device> devs;
  device.sdb_find_by_identity(0x651, 0x4d78adfdU, devs);
  assert (devs.size() == 1);
  address_t ioconf = devs[0].sdb_component.addr_first + 4;
  
  int inputs = 0;
  int connection[MAX_CHANNELS][MAX_CHANNELS];
  memset(connection, 0, sizeof(connection));
  
  /* For each output, generate a pulse chain and confirm it's validity */
  for (int out = 0; out < MAX_CHANNELS; ++out) {
    device.write(ioconf, EB_DATA32, 1 << out);
    /* Generate pulse quickly */
    eca.refresh();
    uint64_t start = eca.time + (CPU_DELAY_MS*1000*1000)/8;
    eca.streams[0].send(EventEntry(0xdeadbeef, 0, 0, start));
    usleep(CPU_DELAY_MS*1000 + (long)(HIGH_NS+LOW_NS)*EVENTS/1000);
    
    /* Read-out result */
    std::vector<std::vector<uint64_t> > queues;
    tlu.pop_all(queues);
    
    assert (queues.size() < MAX_CHANNELS);
    if ((int)queues.size() > inputs) inputs = queues.size();
    
    start <<= 3; // TLU is in 1ns steps, ECA in 8ns steps
    for (int in = 0; in < (int)queues.size(); ++in) {
      std::vector<uint64_t>& queue = queues[in];
      
      /* No source of data? */
      if (queue.empty()) continue;
      
      if (queue.size() != EVENTS) {
        fprintf(stderr, "Input %d did not record %d events from output %d!\n", in, EVENTS, out);
        return 1;
      }
      
      /* Now test phase quality */
      int64_t phase[EVENTS];
      int64_t avg = 0;
      for (int event = 0; event < EVENTS; ++event) {
        uint64_t target = (event%8) + start + (HIGH_NS+LOW_NS)*event;
        phase[event] = queue[event] - target;
        avg += phase[event];
      }
      avg /= EVENTS;
      
      // printf("Phase offset from %d=>%d: %dns\n", out, in, (int)avg);
      for (int event = 0; event < EVENTS; ++event) {
        if (phase[event] > avg + 1 ||
            phase[event] + 1 < avg) {
          fprintf(stderr, "Too much phase noise on %d=>%d! %dns differs too far from average %dns\n",
            out, in, (int)phase[event], (int)avg);
          for (int event = 0; event < EVENTS; ++event)
            printf("Phase: %d\n", (int)phase[event]);
          exit(1);
        }
      }
      
      connection[out][in] = avg + TAG_BIT;
    }
  }
  
  /* Confirm that all inputs were tested */
  int ret = 0;
  for (int in = 0; in < inputs; ++in) {
    int ok = 0;
    printf("Input %d <=", in);
    for (int out = 0; out < MAX_CHANNELS; ++out) {
      if (connection[out][in]) {
        ok = 1;
        printf(" %d (%dns)", out, connection[out][in] - TAG_BIT);
      }
    }
    if (!ok) {
      printf(" nothing connected! - Test invalid.");
      ret = 1;
    }
    
    printf("\n");
  }
  
  if (!ret) {
    printf(">>> test successful <<<\n");
  } else {
    printf("!!! test failed !!!\n");
  }
  
  return ret;
}
