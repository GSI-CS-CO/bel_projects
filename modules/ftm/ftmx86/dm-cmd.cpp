#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>

#include "carpeDM.h"
//#include "node.h"
//#include "block.h"
//#include "minicommand.h"
#include "dotstr.h"
#include "strprintf.h"
#include "filenames.h"

#include <typeinfo>

namespace dnt = DotStr::Node::TypeVal;


static void help(const char *program) {
  fprintf(stderr, "\ndm-cmd v%s, build date %s\nSends a command or dotfile of commands to the DM\nThere are global, local and queued commands\n", TOOL_VER, BUILD_DATE);
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> <command> [target node] [parameter] \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nGeneral Options:\n");
  fprintf(stderr, "  -c <cpu-idx>              Select CPU core by index, default is 0\n");
  fprintf(stderr, "                            <cpu-idx> is used as a bit mask if hex number. Example: 0x0c represents cpu 2 and 3.\n");
  fprintf(stderr, "  -t <thread-idx>           Select thread inside selected CPU core by index, default is 0.\n");
  fprintf(stderr, "                            <thread-idx> is used as a bit mask if hex number. Example: 0xf0 represents thread 4 to 7.\n");
  fprintf(stderr, "  -v                        Verbose operation, print more details\n");
  fprintf(stderr, "  -d                        Debug operation, print everything\n");
  fprintf(stderr, "  -i <command .dot file>    Run commands from dot file\n");
  fprintf(stderr, "  status                    Show status of all threads and cores (default)\n");
  fprintf(stderr, "  details                   Show time statistics and detailed information on uptime and recent changes\n");
  fprintf(stderr, "  clearstats                Clear all status and statistics info\n");
  fprintf(stderr, "  gathertime <Time / ns>    Set msg gathering time for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> Set maximum messages in a packet for priority queue\n");
  fprintf(stderr, "  running                   Show bitfield of all running threads on this CPU core\n");
  fprintf(stderr, "  heap                      Show current scheduler heap\n");
  fprintf(stderr, "  startpattern <pattern>    Request start of selected pattern\n");
  fprintf(stderr, "  abortpattern <pattern>    Try to immediately abort selected pattern\n");
  fprintf(stderr, "  chkrem       <pattern>    Check if removal of selected pattern would be safe\n");
  fprintf(stderr, "  starttime <Time / ns>     Set start time for this thread\n");
  fprintf(stderr, "  preptime <Time / ns>      Set preparation time (lead) for this thread\n");
  fprintf(stderr, "  deadline                  Show next deadline for this thread\n");
  fprintf(stderr, "  origin <target>           Set the node with which selected thread will start\n");
  fprintf(stderr, "  origin                    Return the node with which selected thread will start\n");
  fprintf(stderr, "  cursor                    Show name of currently active node of selected thread\n");
  fprintf(stderr, "  hex <target>              Show hex dump of selected Node \n");
  fprintf(stderr, "  queue <target>            Show content of all queues\n");
  fprintf(stderr, "  rawqueue <target>         Dump all meta information of the command queues of the block <target> including commands\n");
  fprintf(stderr, "  start                     Request start of selected thread. Requires a valid origin.\n");
  //fprintf(stderr, "  stop                      Request stop of selected thread. Does reverse lookup of current pattern, prone to race condition\n");
  //fprintf(stderr, "  cease                     Cease thread at pattern end.\n");
  fprintf(stderr, "  abort                     Immediately abort selected thread.\n");
  fprintf(stderr, "  halt                      Immediately aborts all threads on all CPUs.\n");
  fprintf(stderr, "  lock <target>             Locks all queues of a block for asynchronous queue manipulation mode. This makes the queues invisible to the DM and allowing modification during active runtime.\n");
  fprintf(stderr, "                            ACTIVE LOCK MEANS DM WILL NEITHER WRITE TO NOR READ FROM THIS BLOCK'S QUEUES!\n");
  fprintf(stderr, "  clear <target>            Clears all queues of a locked block allowing modification/refill during active runtime.\n");
  fprintf(stderr, "  unlock <target>           Unlocks all queues of a block, making them visible to the DM.\n");
  //fprintf(stderr, "  force                     Force cursor to match origin\n");
  fprintf(stderr, "  asyncflush <target>  <prios>                  Flushes all pending commands of given priorities (3b Hi-Md-Lo -> 0x0..0x7) in an locked block of the schedule\n");
  fprintf(stderr, "  unlock <target>                               Unlocks a block from asynchronous queue manipulation mode\n");
  fprintf(stderr, "  showlocks                                     Lists all currently locked blocks\n");
  fprintf(stderr, "  staticflush <target> <prios>                  Flushes all pending commands of given priorities (3b Hi-Md-Lo -> 0x0..0x7) in an inactive (static) block of the schedule\n");
  fprintf(stderr, "  staticflushpattern <pattern> <prios>          Flushes all pending commands of given priorities (3b Hi-Md-Lo -> 0x0..0x7) in an inactive (static) pattern of the schedule\n");
  fprintf(stderr, "  rawvisited [<target>]    Show 1 for a visited node, 0 for not visited. If no target node is given, show all nodes.\n");
  fprintf(stderr, "\nQueued commands (viable options in square brackets):\n");
  fprintf(stderr, "  stop <target>                        [laps]   Request stop at selected block (flow to idle). Block must have low prio queue.\n");
  fprintf(stderr, "  stoppattern  <pattern>               [laps]   Request stop of selected pattern\n");
  fprintf(stderr, "  noop <target>                        [lapq]   Placeholder to stall succeeding commands, has no effect itself\n");
  fprintf(stderr, "  flow <target> <destination node>     [lapqs]  Changes schedule flow to <Destination Node>\n");
  fprintf(stderr, "  flowpattern <target pat.> <dst pat.> [lapqs]  Changes schedule flow to <Destination Pattern>\n");
  fprintf(stderr, "  relwait <target> <wait time / ns>    [laps]   Changes Block period to <wait time>\n");
  fprintf(stderr, "  abswait <target> <wait time / ns>    [lap]    Stretches Block period until <wait time>\n");
  fprintf(stderr, "  flush <target> <prios>               [lap]    Flushes all pending commands of given priorities (3b, 0x0..0x7) le cmd priority\n");
  fprintf(stderr, "Options for queued commands:\n");
  fprintf(stderr, "  -l <Time / ns>           Time in ns after which the command will become active, default is 0 (immediately)\n");
  fprintf(stderr, "  -a                       Interprete valid time of command as absolute. Default is relative (current WR time is added)\n");
  fprintf(stderr, "  -p <priority>            The priority of the command (0 = Low, 1 = High, 2 = Interlock), default is 0\n");
  fprintf(stderr, "  -q <quantity>            The number of times the command will be inserted into the target queue, default is 1\n");
  fprintf(stderr, "  -s                       Changes to the schedule are permanent\n");
  fprintf(stderr, "\nDiagnostics:\n");
  fprintf(stderr, "  diag                               Show time statistics and detailed information on uptime and recent changes\n");
  fprintf(stderr, "  cleardiag                          Clears all CPU and HW statistics and details \n");
  fprintf(stderr, "  cfghwdiag <TAI / ns> <Stall / ns>  Sets observation window for ECA TAI time continuity and CPU stall streaks\n");
  fprintf(stderr, "  starthwdiag                        Starts HW diagnostic data acquisition\n");
  fprintf(stderr, "  stophwdiag                         Stops HW diagnostic data acquisition\n");
  fprintf(stderr, "  cfgcpudiag <Warn. Threshold / ns>  Globally sets warning threshold for minimum message dispatch lead\n");
  fprintf(stderr, "  clearcpudiag                       Clears CPU statistics for given index\n");
  fprintf(stderr, "\n");

}

std::string formatTime(uint64_t t) {
  std::string temp = nsTimeToDate(t);
  int length = temp.length();
  std::string zeros = ".000000000";
  // Use the timestamp part (first 19 characters), add a dot and leading zeros
  // for the nanoseconds and the digits for the nanoseconds.
  std::string temp1 = temp.substr(0,19) + zeros.substr(0, 10 - (length - 23)) + temp.substr(20, length - 23);
  //~ std::cout << temp << ", " << length << ", " << temp1 << std::endl;
  return temp1;
}

void showStatus(const char *netaddress, CarpeDM& cdm, bool verbose) {
  std::string show;
  cdm.showMemSpace();
  if(cdm.isOptimisedS2R()) cdm.dirtyCtShow();
  uint8_t cpuQty = cdm.getCpuQty();
  uint8_t thrQty = cdm.getThrQty();

  std::vector<std::string> vsCursor;
  std::vector<std::string> vsCursorPattern;
  std::vector<std::string> vsOrigin;
  std::vector<std::string> vsOriginPattern;
  std::vector<uint64_t> vsMsgCnt;

  //do this fast to get a most coherent picture, no output
  for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for (uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      vsCursor.push_back(cdm.getThrCursor(cpuIdx, thrIdx));
      vsMsgCnt.push_back(cdm.getThrMsgCnt(cpuIdx, thrIdx));
    }
  }

  for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for (uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      vsCursorPattern.push_back(cdm.getNodePattern(vsCursor[cpuIdx * thrQty + thrIdx]));
      vsOrigin.push_back(cdm.getThrOrigin(cpuIdx, thrIdx));
      vsOriginPattern.push_back(cdm.getNodePattern(vsOrigin[cpuIdx * thrQty + thrIdx]));
    }
  }

  // In verbose mode, origin and origin pattern is displayed. This needs 55 + 3 chars each.
  const uint16_t width = 149 + (verbose ? 2 * 58 : 0);
  //this is horrible code, but harmless. Does the job for now.
  //TODO: replace this with something more sensible

  uint64_t timeWrNs = cdm.getDmWrTime();

  printf("\n\u2554"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2557\n");
  const char* FORMAT_STRING = (verbose ?
                  "\u2551 DataMaster: %-77s \u2502 ECA-Time: 0x%08x%08x ns \u2502 %-135.19s   \u2551\n" :
                  "\u2551 DataMaster: %-77s \u2502 ECA-Time: 0x%08x%08x ns \u2502 %.19s   \u2551\n");
  printf(FORMAT_STRING, netaddress, (uint32_t)(timeWrNs>>32), (uint32_t)timeWrNs, formatTime(timeWrNs).c_str() );
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  // print the column headers
  if (verbose) {
    printf("\u2551 %3s \u2502 %3s \u2502 %7s \u2502 %9s \u2502 %55s \u2502 %55s \u2502 %55s \u2502 %55s \u2551\n", "Cpu", "Thr", "Running", "MsgCount", "Pattern", "Node", "Origin pattern", "Origin");
  } else {
    printf("\u2551 %3s \u2502 %3s \u2502 %7s \u2502 %9s \u2502 %55s \u2502 %55s \u2551\n", "Cpu", "Thr", "Running", "MsgCount", "Pattern", "Node");
  }
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");

  bool toggle=false;

  // print one line for each thread on each CPU in verbose mode. Otherwise only running threads.
  for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for (uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      if (verbose || ((cdm.getThrRun(cpuIdx) >> thrIdx) & 1)) {
        //if (!first) {printf("\u2560"); for (int i=0;i<width;i++) printf("\u2500"); printf("\u2563\n");
        std::string running = (((cdm.getThrRun(cpuIdx) >> thrIdx) & 1) ? std::string(KGRN) + std::string("yes") : std::string(KRED) + std::string(" no")) + std::string(KNRM);
        const char* FORMAT_STRING2 = (verbose ?
                        "\u2551%s %2u  \u2502 %2u  \u2502   %3s%s   \u2502 %9llu \u2502 %55s \u2502 %55s \u2502 %55s \u2502 %55s %s\u2551\n" :
                        "\u2551%s %2u  \u2502 %2u  \u2502   %3s%s   \u2502 %9llu \u2502 %55s \u2502 %55s %s\u2551\n");
        printf(FORMAT_STRING2, (toggle ? BLGR : ""), cpuIdx, thrIdx, running.c_str(),
          (toggle ? BLGR : ""),
          (unsigned long long int)vsMsgCnt[cpuIdx * thrQty + thrIdx],
          vsCursorPattern[cpuIdx * thrQty + thrIdx].c_str(),
          vsCursor[cpuIdx * thrQty + thrIdx].c_str(),
          (verbose ? vsOriginPattern[cpuIdx * thrQty + thrIdx].c_str() : ""),
          (verbose ? vsOrigin[cpuIdx * thrQty + thrIdx].c_str() : ""),
          BNRM
        );
        toggle = !toggle;
      }
    }
  }

  printf("\u255A"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u255D\n");

}

void showRawStatus(const char *netaddress, CarpeDM& cdm) {
  uint8_t cpuQty = cdm.getCpuQty();
  uint8_t thrQty = cdm.getThrQty();

  std::vector<std::string> vsCursor;
  std::vector<std::string> vsCursorPattern;
  std::vector<std::string> vsOrigin;
  std::vector<std::string> vsOriginPattern;
  std::vector<uint64_t> vsMsgCnt;

  //do this fast to get a most coherent picture, no output
  for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for (uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      vsCursor.push_back(cdm.getThrCursor(cpuIdx, thrIdx));
      vsMsgCnt.push_back(cdm.getThrMsgCnt(cpuIdx, thrIdx));
    }
  }

  for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for (uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      vsCursorPattern.push_back(cdm.getNodePattern(vsCursor[cpuIdx * thrQty + thrIdx]));
      vsOrigin.push_back(cdm.getThrOrigin(cpuIdx, thrIdx));
      vsOriginPattern.push_back(cdm.getNodePattern(vsOrigin[cpuIdx * thrQty + thrIdx]));
    }
  }

  for (uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for (uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      printf("CPU:%02u,THR:%02u,RUN:%1u\nMSG:%09llu\nPAT:%s,NOD:%s, Origin:%s, OriginPattern:%s\n", cpuIdx, thrIdx, (cdm.getThrRun(cpuIdx) >> thrIdx) & 1,
        (unsigned long long int)vsMsgCnt[cpuIdx * thrQty + thrIdx],
        vsCursorPattern[cpuIdx * thrQty + thrIdx].c_str(),
        vsCursor[cpuIdx * thrQty + thrIdx].c_str(),
        vsOriginPattern[cpuIdx * thrQty + thrIdx].c_str(),
        vsOrigin[cpuIdx * thrQty + thrIdx].c_str()
      );
    }
  }
}

void showHealth(const char *netaddress, CarpeDM& cdm, bool verbose) {
  std::string show;
  uint8_t cpuQty = cdm.getCpuQty();

  HealthReport *hr = new HealthReport[cpuQty];

  HwDelayReport hwdr;


  for (uint8_t i=0; i < cpuQty; i++) { cdm.getHealth(i, hr[i]); }
  cdm.getHwDelayReport(hwdr);

  const uint16_t width = 80;

  //this is horrible code, but harmless. Does the job for now.
  //TODO: replace this with something more sensible

  std::string tsFormat = "%-31s"; // format of timestamps with nanoseconds, left-justified

  uint64_t timeWr = cdm.getDmWrTime();

  unsigned netStrLen;
  for (netStrLen = 0; netStrLen < width; netStrLen++) {if (netaddress[netStrLen] == '\00') break;}

  printf("\n\u2554"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2557\n");
  printf("\u2551 DataMaster: %30s %40s", netaddress, "\u2551\n");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 ECA-Time: 0x%08x%08x          \u2502 ") + tsFormat + std::string(" %10s\n")).c_str(),
          (uint32_t)(timeWr>>32), (uint32_t)timeWr, formatTime(timeWr).c_str(), "\u2551");


  // Boot Time and Msg Count
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %3s \u2502 ") + tsFormat + std::string(" \u2502 %14s \u2502 %10s \u2502%10s\n")).c_str(),
      "Cpu", "BootTime", "CPU Msg Cnt", "State", "Bad W-Time\u2551");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for (uint8_t i=0; i < cpuQty; i++) {
    printf((std::string("\u2551 %3u \u2502 ") + tsFormat + std::string(" \u2502 %14llu \u2502 0x%08x \u2502 %8u %2s\n")).c_str(),
            hr[i].cpu, formatTime(hr[i].bootTime).c_str(), (long long unsigned int)hr[i].msgCnt, hr[i].stat, hr[i].badWaitCnt, "\u2551");
  }

  // Most recent schedule modification (time, issuer, type of operation)
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %3s \u2502 ") + tsFormat + std::string(" \u2502 %8s \u2502 %8s \u2502 %16s %1s\n")).c_str(),
          "Cpu",  "Schedule ModTime", "Issuer", "Host", "Op Type", "\u2551");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for (uint8_t i=0; i < cpuQty; i++) {
    printf((std::string("\u2551 %3u \u2502 ") + tsFormat + std::string(" \u2502 %8s \u2502 %8s \u2502 %16s %1s\n")).c_str(),
            hr[i].cpu, formatTime(hr[i].smodTime).c_str(), hr[i].smodIssuer, hr[i].smodHost, hr[i].smodOpType.c_str(), "\u2551");
  }

  // Most recent command (time, issuer, type of operation)
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %3s \u2502 ") + tsFormat + std::string(" \u2502 %8s \u2502 %8s \u2502 %16s %1s\n")).c_str(),
          "Cpu",  "Command ModTime", "Issuer", "Host", "Op Type", "\u2551");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for (uint8_t i=0; i < cpuQty; i++) {
    printf((std::string("\u2551 %3u \u2502 ") + tsFormat + std::string(" \u2502 %8s \u2502 %8s \u2502 %16s \u2551\n")).c_str(),
            hr[i].cpu, formatTime(hr[i].cmodTime).c_str(), hr[i].cmodIssuer, hr[i].cmodHost, hr[i].cmodOpType.c_str());
  }

  //LM32 message ispatch statistics (min lead, max lead, avg lead, lead warning threshold, warning count, status register)
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 %3s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %12s \u2551\n",
      "Cpu", "Min dT", "Max dT", "Avg dT", "Thrs dT", "Warnings", "Max Backlog");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for (uint8_t i=0; i < cpuQty; i++) {
    printf("\u2551 %3u \u2502 %9d \u2502 %9d \u2502 %9d \u2502 %9d \u2502 %9u \u2502 %12u \u2551\n",
      hr[i].cpu,
      (int)hr[i].minTimeDiff,
      (int)hr[i].maxTimeDiff,
      (int)hr[i].avgTimeDiff,
      (int)hr[i].warningThreshold,
      hr[i].warningCnt,
      hr[i].maxBacklog);
  }
  //
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %3s \u2502 ") + tsFormat + std::string(" \u2502 %-38s \u2551\n")).c_str(), "Cpu",  "1st Warning", "Location");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");

  for (uint8_t i=0; i < cpuQty; i++) {
    printf((std::string("\u2551 %3u \u2502 ") + tsFormat + std::string(" \u2502 %38s \u2551\n")).c_str(),
            hr[i].cpu, formatTime(hr[i].warningTime).c_str(), hr[i].warningNode.c_str());
  }

  const std::string filler = std::string("                       \u2551\n");
  // Hardware Delay Report
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %10s \u2502 %9s \u2502 ") + tsFormat + filler).c_str(), "T observ",  "maxPosDif", "MaxPosUpdate");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %10llu \u2502 %9lld \u2502 ") + tsFormat + filler).c_str(),
    (unsigned long long)hwdr.timeObservIntvl, (signed long long)hwdr.timeMaxPosDif, formatTime(hwdr.timeMaxPosUDts).c_str());
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %10s \u2502 %9s \u2502 ") + tsFormat + filler).c_str(), "T observ", "minNegDif", "minNegUpdate");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %10llu \u2502 %9lld \u2502 ") + tsFormat + filler).c_str(),
    (unsigned long long)hwdr.timeObservIntvl, (signed long long)hwdr.timeMinNegDif, formatTime(hwdr.timeMinNegUDts).c_str());

  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf((std::string("\u2551 %3s \u2502 %10s \u2502 %10s \u2502 %10s \u2502 ") + tsFormat + std::string("   \u2551\n")).c_str(),
      "Cpu", "STL obs.", "MaxStreak",  "Current", "MaxStreakUpdate", "");
  printf("\u2560"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for (uint8_t i=0; i < cpuQty; i++) {
    printf((std::string("\u2551 %3u \u2502 %10u \u2502 %10u \u2502 %10u \u2502 ") + tsFormat + std::string("   \u2551\n")).c_str(),
      (int)i,
      hwdr.stallObservIntvl,
      hwdr.sdr[i].stallStreakMax,
      hwdr.sdr[i].stallStreakCurrent,
      formatTime(hwdr.sdr[i].stallStreakMaxUDts).c_str());
  }

  printf("\u255A"); for (int i=0;i<width;i++) printf("\u2550"); printf("\u255D\n");
}

std::string get_working_path() {
   char temp[MAXPATHLEN];
   return ( getcwd(temp, sizeof(temp)) ? std::string( temp ) : std::string("") );
}

uint32_t getBitMask(const char *optarg, const uint32_t quantity, const char *program, int *error, const char *text) {
  /* This method works for quantity < 64. Otherwise bitMask has an overflow.
   */
  assert(quantity < 64);
  uint64_t bitMask = (1ll << quantity) - 1;
  uint32_t bits = bitMask;
  uint64_t tmp;
  if (optarg == NULL) {
    *error = -1;
    return 0;
  } else {
    bool isNumber = true;
    if (strlen(optarg) > 1 && optarg[0] == '0' && (optarg[1] == 'x' || optarg[1] == 'X')) {
      tmp = strtoll(optarg, NULL, 0);
      if ((tmp & ~bitMask) != 0) {
        std::cerr << program << ": " << text << " mask '" << optarg << "' is invalid. Choose a mask that fits to 0x" << std::hex << bitMask << "." << std::endl;
        *error = -1;
      } else {
        bits = (uint32_t) tmp;
      }
    } else {
      for (size_t i=0; isNumber && i < strlen(optarg); i++) {
        isNumber = isdigit(optarg[i]);
      }
      if (isNumber) {
        tmp = strtol(optarg, NULL, 0);
        if ((tmp < 0) || (tmp >= quantity)) {
          std::cerr << program << ": " << text << " idx '" << optarg << "' is invalid. Choose an index between 0 and " << quantity -1 << "." << std::endl;
          *error = -1;
        } else {
          bits = (1 << (uint32_t)tmp);
        }
      } else {
        std::cerr << program << ": " << text << " argument '" << optarg << "' is invalid. Not a number." << std::endl;
        *error = -1;
      }
    }
    return bits;
  }
}

typedef vEbwrs& (CarpeDM::*setterFunction)(vEbwrs& ew, uint8_t cpuIdx, uint8_t thrIdx, uint64_t t);
typedef uint64_t (CarpeDM::*getterFunction)(uint8_t cpuIdx, uint8_t thrIdx);

bool handleCpuBitsThreadBits(const char *text, uint32_t cpuBits, uint32_t threadBits, setterFunction setter, getterFunction getter, CarpeDM& cdm, std::string *targetName, vEbwrs& ew) {
  if (setter != nullptr && !targetName->empty()) {
    uint64_t parameter = std::stoll(*targetName, nullptr, 0);
    for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
      if ((cpuBits >> cpu) & 1) {
        for (int thread=0; thread < cdm.getThrQty(); thread++) {
          if ((threadBits >> thread) & 1) {
            (cdm.*setter)(ew, cpu, thread, parameter);
            std::cout << std::dec << "setting " << text << ": CPU " << cpu << " Thread " << thread << "." << std::endl;
          }
        }
      }
    }
    // no return here, next action: send commands with ew vector.
    return false;
  } else {
    for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
      if ((cpuBits >> cpu) & 1) {
        for (int thread=0; thread < cdm.getThrQty(); thread++) {
          if ((threadBits >> thread) & 1) {
            std::string caption = std::string(text);
            caption[0] = std::toupper(caption[0]);
            std::cout << std::dec << "CPU " << cpu << " Thr " << thread << " " << caption << " " << (cdm.*getter)(cpu, thread) << std::endl;
          }
        }
      }
    }
    return true;
  }
  return false;
}

int main(int argc, char* argv[]) {

  bool verbose = false, permanent = false, debug=false, vabs=false, force=false;
  bool setThreadBits = false;
  bool setCpuBits = false;

  int opt;
  const char *program = argv[0];
  const char cTypeName[] = "status";
  const char *netaddress, *cmdFilename = NULL, *typeName = (char*)&cTypeName, *para = NULL;
  std::string targetName = {};
  std::string dirname = get_working_path();

  int32_t tmp;
  int32_t error=0;
  //~ uint32_t cpuIdx = 0;
  uint32_t cmdPrio = PRIO_LO;
  uint32_t cmdQty = 1;
  uint64_t cmdTvalid = 0, longtmp;
  uint32_t threadBits = 0x1;
  const char *tempThreadBits = NULL;
  uint32_t cpuBits = 0x1;
  const char *tempCpuBits = NULL;

// start getopt
   while ((opt = getopt(argc, argv, "shvc:p:l:t:q:i:daf")) != -1) {
      switch (opt) {
        case 'f':
          force = true;
          break;
        case 'a':
          vabs = true;
          break;
        case 'i':
          cmdFilename = optarg;
          break;
        case 'd':
          debug = true;
          break;
        case 'v':
          verbose = 1;
          break;
        case 't':
          tempThreadBits = optarg;
          setThreadBits = true;
          break;
        case 'c':
          tempCpuBits = optarg;
          setCpuBits = true;
          break;
        case 'l':
          longtmp = strtoll(optarg, NULL, 0);
          if (longtmp < 0) {
            std::cerr << program << ": Valid time must be a positive offset of nanoseconds to UTC 0 (12:00 Jan 1st 1970)" << std::endl;
            error = -1;
          } else {
            cmdTvalid = (uint64_t)longtmp;
          }
          break;
        case 'p':
          tmp = strtol(optarg, NULL, 0);
          if ((tmp < PRIO_LO) || (tmp > PRIO_IL)) {
            std::cerr << program << ": Priority must be 0 (Low), 1 (High) or Interlock (2)  -- '" << std::endl;
             error = -1;
          } else {
            cmdPrio = (uint32_t)tmp;
          }
          break;
        case 'q':
          tmp = strtol(optarg, NULL, 0);
          if ((tmp < 0) || (tmp > ACT_QTY_MSK)) {
            std::cerr << program << ": Command quantity must be between 0 and " << ACT_QTY_MSK << std::endl;
            error = -1;
          } else {
            cmdQty = (uint32_t)tmp;
          }
          break;
        case 's':
          permanent = true;
          break;
        case 'h':
          help(program);
          return 0;
        case ':':
        case '?':
          error = -2;
          break;
        default:
          std::cerr << program << ": bad getopt result" << std::endl;
          error = -3;
      }
   }


  if (error) return error;
  /*
  if (optind+1 >= argc && cmdFilename == NULL) {

   std::cerr << program << ": expecting two non-optional arguments + command: <etherbone-device> <command> " << std::endl;
    //help();
    return -4;
  }
  */

  if (optind+0 >= argc) {
    std::cerr << program << ": expecting one non-optional arguments: <etherbone-device>" << std::endl;
    //help();
    return -4;
  }

  // process command arguments

  netaddress = argv[optind];

  if (optind+1 < argc) typeName        = argv[optind+1];
  if (optind+2 < argc) targetName      = std::string(argv[optind+2]);
  if (optind+3 < argc) para            = argv[optind+3];

  CarpeDM cdm;

  if(verbose) cdm.verboseOn();
  if(debug)   cdm.debugOn();

  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
    std::cerr << program << ": Could not connect to DM. Cause: " << err.what() << std::endl; return -20;
  }

  // evaluate the bit mask for the CPUs after connecting to firmware. Otherwise getCpuQty() is not valid.
  if (setCpuBits) {
    uint32_t uTemp = getBitMask(tempCpuBits, cdm.getCpuQty(), program, &error, "Cpu");
    if (error == 0) {
      cpuBits = uTemp;
    } else {
      return error;
    }
  }
  if (setThreadBits) {
    uint32_t uTemp = getBitMask(tempThreadBits, cdm.getThrQty(), program, &error, "Thread");
    if (error == 0) {
      threadBits = uTemp;
    } else {
      return error;
    }
  }

  for (int cpu=0; cpu < cdm.getCpuQty(); cpu++) {
    if ((cpuBits >> cpu) & 1) {
      if (!(cdm.isCpuIdxValid(cpu))) {
        std::cerr << program << ": CPU Idx " << cpu << " does not refer to a CPU with valid firmware." << std::endl << std::endl;
        return -30;
      }
    }
  }

  cdm.updateModTime();

  vEbwrs ew;
  cdm.lockManagerClear();

  // The global hard abort commands are special - they must work regardless if the schedule download/parse was successful or not
  if ((typeName != NULL )  && ( typeName != std::string(""))) {

    std::string tmpGlobalCmds(typeName);

    if (tmpGlobalCmds == dnt::sCmdAbort)  {
      if (!targetName.empty()) {
        uint32_t bits = std::stol(targetName, nullptr, 0);
        for (int cpu=0; cpu < cdm.getCpuQty(); cpu++) {
          if ((cpuBits >> cpu) & 1) {
            cdm.setThrAbort(ew, cpu, bits & ((1ll<<cdm.getThrQty())-1));
            std::cout << "CPU " << cpu << " Threads 0x" << std::hex << bits << " set for abort." << std::endl;
          }
        }
      } else {
        for (int cpu=0; cpu < cdm.getCpuQty(); cpu++) {
          if ((cpuBits >> cpu) & 1) {
            for (int thread=0; thread < cdm.getThrQty(); thread++) {
              if ((threadBits >> thread) & 1) {
                cdm.abortThr(ew, cpu, thread);
                //~ std::cout << "CPU " << cpu << " Thread " << thread << " aborted. " << std::bitset<8>{threadBits} << std::endl;
                std::cout << "CPU " << cpu << " Thread " << thread << " aborted." << std::endl;
              }
            }
          }
        }
      }
    } else if (tmpGlobalCmds == "halt") {
      cdm.halt();
      return 0;
    } else if (tmpGlobalCmds== "reset") {
      bool clearStatistic = targetName == std::string("all");
      std::cout << "clear statistic:" << std::boolalpha << clearStatistic << std::endl;
      cdm.softwareReset(clearStatistic);
      return 0;
    }
  }

  try {
    cdm.download();
  } catch (std::runtime_error const& err) {
    std::cerr << program << ": Download failed. Cause: " << err.what() << std::endl;
    return -7;
  }

  for (int cpu=0; cpu < cdm.getCpuQty(); cpu++) {
    if ((cpuBits >> cpu) & 1) {
      uint32_t globalStatus = cdm.getStatus(0), status = cdm.getStatus(cpu);
      if (!force && ( !(globalStatus & (SHCTL_STATUS_EBM_INIT_SMSK | SHCTL_STATUS_PQ_INIT_SMSK))
        || !(status & (SHCTL_STATUS_UART_INIT_SMSK | SHCTL_STATUS_DM_INIT_SMSK)) ))
        {
        std::cerr << program << ": DM is not fully initialised. Cause: " << std::endl;
        if (!(globalStatus & SHCTL_STATUS_EBM_INIT_SMSK)) std::cerr << "EB Master could not be configured. Does the DM have a valid IP?" << std::endl;
        if (!(globalStatus & SHCTL_STATUS_PQ_INIT_SMSK))  std::cerr << "Priority Queue could not be configured" << std::endl;
        if (!(status & SHCTL_STATUS_UART_INIT_SMSK))      std::cerr << "CPU " << cpu << "'s UART is not functional" << std::endl;
        if (!(status & SHCTL_STATUS_DM_INIT_SMSK))        std::cerr << "CPU " << cpu << " could not be initialised" << std::endl;
        //if (!(status & SHCTL_STATUS_WR_INIT_SMSK))      std::cerr << "CPU " << cpu << "'s WR time could not be initialised" << std::endl;
        return -40;
      }
    }
  }

  //check if we got a dot full of commands and send if so
  if (cmdFilename != NULL) {
    try {
      cdm.sendCommandsDotFile(cmdFilename);
    } catch (std::runtime_error const& err) {
      std::cerr << program << ": Could not send command .dot " << cmdFilename << ". Cause: " << err.what() << std::endl;
    }
    return 0;
  }

  uint64_t tvalidOffs = cdm.getModTime();
  //~ std::cout << program << ": tvalidOffs: " << tvalidOffs << ", cmdTvalid: " << cmdTvalid << std::endl;
  if(!vabs) cmdTvalid += tvalidOffs; // already added modTime when !vabs, so when calling cdm.adjustValidTime, we'll always say tvalid is absolute
  //~ std::cout << program << ": tvalidOffs: " << tvalidOffs << ", cmdTvalid: " << cmdTvalid << std::endl;

  if ((typeName != NULL ) && ( typeName != std::string(""))) {
    if (verbose) {
      std::cout << "Generating " << typeName << " command" << std::endl;
    }
    std::string cmp(typeName);
    // For commands with a target name: check that the target name is valid.
    // Vector of all commands which need a target name.
    std::vector<std::string> commands_with_targetName = {
      dnt::sCmdNoop, dnt::sCmdFlow, dnt::sSwitch, "relwait", "abswait", dnt::sCmdFlush,
      "staticflush", "lock", "asyncclear", "unlock", "queue", "rawqueue"
    };
    for (std::string s : commands_with_targetName) {
      // if the command needs a target name, check
      if (cmp == s) {
        if ((targetName.empty())) {
          std::cerr << program << ": Target node is NULL, target missing." << std::endl;
          return -1;
        }
        if(!(cdm.isInHashDict(targetName))) {
          std::cerr << program << ": Target node '" << targetName << "' was not found on DM" << std::endl;
          return -1;
        }
      }
    }

    // Main 'if else if' over all commands
    if (cmp == dnt::sCmdNoop) {
      try {
        // use thread=0 here. The command queue of the block (targetName) is not connected to threads.
        cdm.createQCommand(ew, cmp, targetName, cmdPrio, cmdQty, true, 0, 0);
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Command noop on " << targetName << " failed. Cause: " << err.what() << std::endl;
        return -1;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "status") {
      showStatus(netaddress, cdm, verbose);
      return 0;
    } else if (cmp == "rawstatus") {
      showRawStatus(netaddress, cdm);
      return 0;
    } else if ( (cmp == "details") || (cmp == "diag")) {
      showHealth(netaddress, cdm, verbose);
      return 0;
    } else if (cmp == "flowpattern") {
      if (( targetName.empty()) || (para == NULL) || ( para == std::string(""))) {
        std::cerr << program << ": Need valid target and destination pattern names." << std::endl;
        return -1;
      }
      std::string fromNode = cdm.getPatternExitNode(targetName);
      std::string toNode   = (para == DotStr::Node::Special::sIdle ) ? DotStr::Node::Special::sIdle : cdm.getPatternEntryNode(para);
      if ( cdm.isInHashDict( fromNode ) && ( (toNode == DotStr::Node::Special::sIdle ) || cdm.isInHashDict( toNode )  )) {
        //~ std::cout << program << ": vabs: " << vabs << ": tvalidOffs: " << tvalidOffs << ", cmdTvalid: " << cmdTvalid << std::endl;
        cdm.createFlowCommand(ew, dnt::sCmdFlow, fromNode, toNode, cmdPrio, cmdQty, true, cmdTvalid, permanent);
      } else {
        std::cerr << program << ": Destination Node '" << toNode << "'' was not found on DM." << std::endl;
        return -1;
      }
      targetName = fromNode.c_str();
      // no return here, next action: send commands with ew vector.
    } else if (cmp == dnt::sCmdFlow)  {
      if (( ((para != NULL) && ( para != std::string("")))) && (((para == DotStr::Node::Special::sIdle ) || cdm.isInHashDict( para)))) {
        //~ std::cout << program << ": vabs: " << vabs << ": tvalidOffs: " << tvalidOffs << ", cmdTvalid: " << cmdTvalid << std::endl;
        cdm.createFlowCommand(ew, dnt::sCmdFlow, targetName, para, cmdPrio, cmdQty, true, cmdTvalid, permanent);
      } else {
        std::cerr << program << ": Destination Node '" << para << "' was not found on DM." << std::endl;
        return -1;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "switchpattern")  {
      if (( targetName.empty()) || (para == NULL) || ( para == std::string(""))) {std::cerr << program << ": Need valid target and destination pattern names." << std::endl;
        return -1;
      }
      std::string fromNode = cdm.getPatternExitNode(targetName);
      std::string toNode   = (para == DotStr::Node::Special::sIdle) ? DotStr::Node::Special::sIdle : cdm.getPatternEntryNode(para);
      if (cdm.isInHashDict(fromNode) && ((toNode == DotStr::Node::Special::sIdle) || cdm.isInHashDict(toNode))) {
        // use thread = 0 here since thread is irrelevant for this command.
        cdm.createCommand(ew, dnt::sSwitch, fromNode, toNode, cmdPrio, cmdQty, true, cmdTvalid, permanent,
              false, false, false, 0, false, false, false, 0);
      } else {
        std::cerr << program << ": Destination Node '" << toNode << "' was not found on DM" << std::endl;
        return -1;
      }
      targetName = fromNode.c_str();
      // no return here, next action: send commands with ew vector.
    } else if (cmp == dnt::sSwitch) {
      if (( ((para != NULL) && ( para != std::string("")))) && (((para == DotStr::Node::Special::sIdle ) || cdm.isInHashDict( para)))) {
        // use thread = 0 here since thread is irrelevant for this command.
        cdm.createCommand(ew, dnt::sSwitch, targetName, para, cmdPrio, cmdQty, true, cmdTvalid, permanent,
              false, false, false, 0, false, false, false, 0);
      } else {
        std::cerr << program << ": Destination Node '" << para << "' was not found on DM" << std::endl;
        return -1;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "relwait") {
      if ((para == NULL) || (para == std::string(""))) {
        std::cerr << program << ": Wait time in ns is missing" << std::endl;
        return -1;
      }
      cdm.createWaitCommand(ew, dnt::sCmdWait, targetName, cmdPrio, cmdQty, true, cmdTvalid, strtoll(para, NULL, 0), false);
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "abswait") {
      if ((para == NULL) || (para == std::string(""))) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
      cdm.createWaitCommand(ew, dnt::sCmdWait, targetName, cmdPrio, cmdQty, true, cmdTvalid, strtoll(para, NULL, 0), true);
      // no return here, next action: send commands with ew vector.
    } else if (cmp == dnt::sCmdFlush) {
      if ((para == NULL) || (para == std::string(""))) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }
      uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
      cdm.createFlushCommand(ew, dnt::sCmdFlush, targetName, "", cmdPrio, cmdQty, true, cmdTvalid, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1));
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "staticflush") {
      if ((para == NULL) || (para == std::string(""))) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }
      uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
      try {
        cdm.staticFlushBlock(targetName, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1), force);
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Could not statically flush " << targetName << ". Cause: " << err.what() << std::endl;
      }
      return 0;
    } else if (cmp == "lock") {
      bool wr=true, rd=true;
      if ((para != NULL) && (para != std::string(""))) {
        uint8_t tmp = strtol(para, NULL, 0) & 0x3;
        wr = (bool)(tmp & BLOCK_CMDQ_DNW_SMSK);
        rd = (bool)(tmp & BLOCK_CMDQ_DNR_SMSK);
      }
      try {
        cdm.createLockCtrlCommand(ew, dnt::sCmdLock, targetName, rd, wr);
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Could not lock block " << targetName << ". Cause: " << err.what() << std::endl;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "asyncclear") {
      try {
        for (int thread=0; thread < cdm.getThrQty(); thread++) {
          if ((threadBits >> thread) & 1) {
            cdm.createNonQCommand(ew, dnt::sCmdAsyncClear, targetName, thread);
          }
        }
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Could not clear block " << targetName << "'s queues. Cause: " << err.what() << std::endl;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "unlock") {
      bool wr=true, rd=true;
      if ((para != NULL) && (para != std::string(""))) {
        uint8_t tmp = strtol(para, NULL, 0) & 0x3;
        wr = (bool)(tmp & BLOCK_CMDQ_DNW_SMSK);
        rd = (bool)(tmp & BLOCK_CMDQ_DNR_SMSK);
      }
      try {
        cdm.createLockCtrlCommand(ew, dnt::sCmdUnlock, targetName, rd, wr);
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Could not unlock block " << targetName << ". Cause: " << err.what() << std::endl;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "showlocks") {
      vStrC res;
      try {
        res = cdm.getLockedBlocks(true, true);
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Could not list locked blocks. Cause: " << err.what() << std::endl;
      }
      std::cout << "Locked Blocks: " << res.size() << std::endl;
      for (auto s : res) {
        std::cout << s << std::endl;
      }
      return 0;
    } else if (cmp == "queue") {
        uint64_t timeWrNs = cdm.getDmWrTime();
        std::cout << "Called at: " << formatTime(timeWrNs) << "  " << timeWrNs << std::endl;
        std::string report;
        std::cout << cdm.inspectQueues(targetName, report) << std::endl;
        return 0;
    } else if (cmp == "rawqueue") {
        std::string report;
        std::cout << cdm.getRawQReport(targetName, report) << std::endl;
        return 0;
    } else if (cmp == dnt::sCmdOrigin) {
      if (( !targetName.empty())){
        if(!(cdm.isInHashDict( targetName)) && targetName != DotStr::Node::Special::sIdle) {
          std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl;
          return -1;
        }
        for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
          if ((cpuBits >> cpu) & 1) {
            for (int thread=0; thread < cdm.getThrQty(); thread++) {
              if ((threadBits >> thread) & 1) {
                // set the origin for the first thread in threadBits, then break.
                // possible improvement: error message if more bits are set in threadBits.
                cdm.setThrOrigin(ew, cpu, thread, targetName);
                break;
              }
            }
          }
        }
      }
      if (verbose | (targetName.empty())) {
        for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
          if ((cpuBits >> cpu) & 1) {
            for (int thread=0; thread < cdm.getThrQty(); thread++) {
              if ((threadBits >> thread) & 1) {
                std::cout << "CPU " << cpu << " Thread " << thread << " origin points to node " << cdm.getThrOrigin(cpu, thread) << std::endl;
              }
            }
          }
        }
        return 0;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "cursor") {
      for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
        if ((cpuBits >> cpu) & 1) {
          for (int thread=0; thread < cdm.getThrQty(); thread++) {
            if ((threadBits >> thread) & 1) {
              std::cout << "Currently at " << cdm.getThrCursor(cpu, thread) << std::endl;
            }
          }
        }
      }
      return 0;
    } else if (cmp == "force") {
      for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
        if ((cpuBits >> cpu) & 1) {
          for (int thread=0; thread < cdm.getThrQty(); thread++) {
            if ((threadBits >> thread) & 1) {
              cdm.forceThrCursor(cpu, thread);
            }
          }
        }
      }
      return 0;
    } else if (cmp == "chkrem") {
      try {
        std::string report;
        bool isSafe = cdm.isSafeToRemove(targetName, report);
        cdm.writeTextFile(dirname + "/" + std::string(debugfile), report);
        std::cout << std::endl << "Pattern " << targetName << " content removal: " << (isSafe ? "SAFE" : "FORBIDDEN" ) << std::endl;
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": " << err.what() << std::endl;
        return -21;
      }
      return 0;
    } else if (cmp == dnt::sCmdStart)  {
      //check if a valid origin was assigned before executing
      std::string origin;
      uint32_t bits;
      if ((!targetName.empty())) {
        bits = std::stol(targetName, nullptr, 0);
      } else {
        bits = threadBits;
      }
      for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
        if ((cpuBits >> cpu) & 1) {
          for (int i=0; i < cdm.getThrQty(); i++) {
            if ((bits >> i) & 1) {
              origin = cdm.getThrOrigin(cpu, i);
              if ((origin == DotStr::Node::Special::sIdle) || (origin == DotStr::Misc::sUndefined)) {
                std::cerr << program << ": Cannot start, origin of CPU " << cpu << "'s thread " << i << " is not a valid node" << std::endl;
                return -1;
              }
            }
          }
          cdm.setThrStart(ew, cpu, bits & ((1ll<<cdm.getThrQty())-1));
        }
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == dnt::sCmdStop) {
      if (targetName.empty()) {
        std::cerr << program << ": Target name is missing" << std::endl;
      } else if (!cdm.isInHashDict(targetName)) {
        std::cerr << program << ": Target node '" << targetName << "' was not found on DM" << std::endl;
      } else  {
        try {
          cdm.stopNodeOrigin(targetName);
        } catch (std::runtime_error const& err) {
          std::size_t pos = std::string(err.what()).find("Block node does not have requested queue");
          if (pos != std::string::npos) {
            std::cerr << program << ": Block node '" << targetName << "' does not have a low prio queue." << std::endl;
          } else {
            pos = std::string(err.what()).find("carpeDMcommand: unknown cpu/adr combo");
            if (pos != std::string::npos) {
              std::cerr << program << ": Node '" << targetName << "' is not a block" << std::endl;
            } else {
              std::cerr << program << ": " << err.what() << "." << std::endl;
            }
          }
          return -1;
        }
      }
      return 0;
    } else if (cmp == "startpattern")  {
      //check if a valid origin was assigned before executing
      if (!targetName.empty()) {
        for (int thread=0; thread < cdm.getThrQty(); thread++) {
          if ((threadBits >> thread) & 1) {
            try {
              cdm.startPattern(ew, targetName, thread);
            } catch (std::runtime_error const& err) {
              std::size_t pos = std::string(err.what()).find("HashTable: Name undefined not found");
              if (pos != std::string::npos) {
                std::cerr << program << ": Target '" << targetName << "' is not a pattern name." << std::endl;
              } else {
                std::cerr << program << ": " << err.what() << "." << std::endl;
              }
              return -1;
            }
          }
        }
      } else {
        std::cout << "Missing valid pattern name" << std::endl;
        return -1;
      }
      // no return here, next action: send commands with ew vector.
    } else if (cmp == "stoppattern")  {
      if (!targetName.empty()) {
        try {
          cdm.stopPattern(targetName);
        } catch (std::runtime_error const& err) {
          std::size_t pos = std::string(err.what()).find("getBaseAdr: unknown target  HashTable: Name undefined not found");
          if (pos != std::string::npos) {
            std::cerr << program << ": Target '" << targetName << "' is not a pattern name." << std::endl;
          } else {
            std::cerr << program << ": " << err.what() << "." << std::endl;
          }
          return -1;
        }
      } else {
        std::cout << "Missing valid pattern name" << std::endl;
      }
      return 0;
    } else if (cmp == "abortpattern")  {
      if (!targetName.empty()) {
        cdm.abortPattern(targetName);
      } else {
        std::cout << "Missing valid pattern name" << std::endl;
      }
      return 0;
    } else if (cmp == "staticflushpattern")  {
      if (( !targetName.empty())) {
        if ((para == NULL) || ( para == std::string(""))) {
          std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl;
          return -1;
        }
        uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
        cdm.staticFlushPattern(targetName, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1), force);
      } else {
        std::cout << "Missing valid Pattern name" << std::endl;
      }
      return 0;
    } else if (cmp == "running")  {
      for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
        if ((cpuBits >> cpu) & 1) {
          std::cout << "CPU " << cpu << " Running Threads: 0x" << std::hex << cdm.getThrRun(cpu) << std::endl;
        }
      }
      return 0;
    } else if (cmp == "hex")  {
      try {
        if((cdm.isInHashDict(targetName))) {
          cdm.dumpNode(targetName);
        } else {
          std::cerr << program << ": Target node '" << targetName << "' not found on DM." << std::endl;
          return -1;
        }
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Node not found. Cause: " << err.what() << std::endl; return -21;
      }
      return 0;
    } else if (cmp == "rawvisited")  {
      try {
        if ((targetName.empty())) {
          cdm.showPaint();
        } else {
          if(!(cdm.isInHashDict(targetName))) {
            std::cerr << program << ": Target node '" << targetName << "' was not found on DM" << std::endl;
            return -1;
          }
          bool isVisited = cdm.isPainted(targetName);
          std::cout << targetName << ":" << (int)(isVisited ? 1 : 0) << std::endl;
        }
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Node not found. Cause: " << err.what() << std::endl; return -21;
      }
      return 0;
    } else if (cmp == "heap")  {
      for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
        if ((cpuBits >> cpu) & 1) {
          cdm.inspectHeap(cpu);
        }
      }
      return 0;
    } else if (cmp == "starttime") {
      if (handleCpuBitsThreadBits("starttime", cpuBits, threadBits, &CarpeDM::setThrStartTime, &CarpeDM::getThrStartTime, cdm, &targetName, ew)) {
        return 0;
      }
    } else if (cmp == "preptime") {
      if (handleCpuBitsThreadBits("preptime", cpuBits, threadBits, &CarpeDM::setThrPrepTime, &CarpeDM::getThrPrepTime, cdm, &targetName, ew)) {
        return 0;
      }
    } else if (cmp == "deadline")  {
      if (handleCpuBitsThreadBits("Deadline", cpuBits, threadBits, nullptr, &CarpeDM::getThrDeadline, cdm, &targetName, ew)) {
        return 0;
      }
    } else if (cmp == "cleardiag")  {
      cdm.clearHealth();
      cdm.clearHwDiagnostics();
      return 0;
    } else if (cmp == "clearhwdiag")  {
      cdm.clearHwDiagnostics();
      return 0;
    } else if (cmp == "starthwdiag") {
      cdm.startStopHwDiagnostics(true);
      return 0;
    } else if (cmp == "stophwdiag") {
      cdm.startStopHwDiagnostics(false);
      return 0;
    } else if (cmp == "clearcpudiag")  {
      for (int cpu = 0; cpu < cdm.getCpuQty(); cpu++) {
        if ((cpuBits >> cpu) & 1) {
          cdm.clearHealth(cpu);
        }
      }
      return 0;
    } else if (cmp == "cfghwdiag") {
      if (!targetName.empty() && (para != NULL) && ( para != std::string(""))) {
        cdm.configHwDiagnostics(std::stoll(targetName, nullptr, 0), strtoll(para, NULL, 0));
      } else {
        std::cerr << program << ": Needs valid values for both TAI time observation interval and stall observation interval" << std::endl;
        return -1;
      }
      return 0;
    } else if (cmp == "cfgcpudiag") {
      if (!targetName.empty()) {
        cdm.configFwDiagnostics(std::stoll(targetName, nullptr, 0));
      } else {
        std::cerr << program << ": Needs valid value for lead warning threshold" << std::endl;
        return -1;
      }
      return 0;
    } else if (cmp == "gathertime") {
      if (!targetName.empty()) {
        //setPqTgather(std::stoll(targetName, nullptr, 0));
      } else {
        //uint64_t tGather = getPqTgather();
        std::cerr << program << ": Needs valid value for lead warning threshold" << std::endl;
        return -1;
      }
      return 0;
    } else if (cmp == "maxmsg") {
      if (!targetName.empty()) {
        //uint64_t maxMsg = getPqMaxMsg();
      } else {
        //setPqPqMaxMsg(std::stoll(targetName, nullptr, 0));
        std::cerr << program << ": Needs valid value for lead warning threshold" << std::endl;
        return -1;
      }
      return 0;
    }

    //all the block commands set mc, so...
    //~ std::cout << program << " try sending " << ew.va.size() << ", locks: " << cdm.lockManagerHasEntries() << std::endl;
    if ((ew.va.size() > 0) | cdm.lockManagerHasEntries()) {
      try {
        cdm.send(ew);
        //~ std::cout << program << " sending " << ew.va.size() << "." << std::endl;
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Could not send command " << para << ". Cause: " << err.what() << std::endl;
      }
      return 0;
    }
    std::cerr << program << ": " << cmp << " is not a valid command. Type " << program << " -h for help" << std::endl;
  }
  return 0;
}
