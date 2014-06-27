#include <etherbone.h>
#include <tlu.h>
#include <eca.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h> // sleep

using namespace GSI_ECA;
using namespace GSI_TLU;

void validate(uint64_t start, int in, int out, const std::vector<uint64_t>& queue) {
  /* Now test phase quality */
  int64_t phase[16];
  int64_t avg = 0;
  for (int event = 0; event < 16; ++event) {
    uint64_t target = (event%8) + start + 200000000*event;
    phase[event] = queue[event] - target;
    //printf("Phase: %d\n", (int)phase[event]);
    avg += phase[event];
  }
  avg /= 16;
  
  printf("Phase offset from %d=>%d: %dns\n", out, in, (int)avg);
  for (int event = 0; event < 16; ++event) {
    if (phase[event] > avg + 1 ||
        phase[event] + 1 < avg) {
      fprintf(stderr, "Too much phase noise on %d=>%d! %dns differs too far from average %dns\n",
        out, in, (int)phase[event], (int)avg);
      exit(1);
    }
  }
}

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
  for (int i = 0; i < 16; ++i) {
    /* 100ms high, 100ms low */
    table.add(TableEntry(0xdeadbeef, i*25000000,           0x0000ffffU,             0, 64));
    table.add(TableEntry(0xdeadbeef, i*25000000 + 12500000, 0xffff0000U,             0, 64));
    table.add(TableEntry(0xdeadbeef, i*25000000,           0x00000FFFU + (i << 29), 2, 64));
    table.add(TableEntry(0xdeadbeef, i*25000000 + 12500000, 0x00FFF000U + (i << 29), 2, 64));
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
  
  int cable1 = -1, cable2 = -1;
  
  /* For each output, generate a pulse chain and confirm it's validity */
  for (int out = 0; out < 3; ++out) {
    device.write(ioconf, EB_DATA32, 1 << out);
    /* Generate pulse in 2 seconds */
    eca.refresh();
    uint64_t start = eca.time + 250000000;
    eca.streams[0].send(EventEntry(0xdeadbeef, 0, 0, start));
    sleep(6);
    
    /* Read-out result */
    std::vector<std::vector<uint64_t> > queues;
    tlu.pop_all(queues);
    
    start <<= 3; // TLU is in 1ns steps, ECA in 8ns steps
    for (int in = 0; in < 3; ++in) {
      std::vector<uint64_t>& queue = queues[in];
      
      /* Detect cable */
      if (in != out) {
        if (!queue.empty()) {
          if (cable1 == -1) {
            printf("Detected cable bridging %d-%d\n", in, out);
            cable1 = out;
            cable2 = in;
          } else if ((cable1 != in  && cable2 != in) ||
                     (cable1 != out && cable2 != out)) {
            fprintf(stderr, "Impossible! Cable connects more than two IOs %d-%d and %d-%d\n",
              cable1, cable2, in, out);
            return 1;
          }
        } else {
          if ((cable1 == in  || cable2 == in) &&
              (cable1 == out || cable2 == out)) {
            fprintf(stderr, "Direction check failure! IO %d=>%d works, but not %d=>%d\n",
              cable1, cable2, out, in);
            return 1;
          }
          continue;
        }
      }
      
      if (queue.size() != 16) {
        fprintf(stderr, "Input %d did not record 16 events from output %d!\n", in, out);
        return 1;
      }
      
      validate(start, in, out, queue);
    }
    
    for (int lvds = 3; lvds < 5; ++lvds) {
      std::vector<uint64_t>& queue = queues[lvds];
      
      if (queue.empty()) {
        fprintf(stderr, "!!! warning: failed to detect cable on LVDS input\n");
        continue;
      }
      
      if (queue.size() != 16) {
        fprintf(stderr, "Input %d did not record 16 events!\n", lvds);
        return 1;
      }
      validate(start, lvds, -1, queue);
    }
  }
  
  if (cable1 == -1) {
    fprintf(stderr, "Failed to detect loopback cable!\n");
    return 1;
  }
  
  printf(">>> testing complete <<<\n");
  return 0;
}
