#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <time.h> 

#include "ftm_shared_mmap.h"
#include "carpeDM.h"
#include "node.h"
#include "block.h"
#include "minicommand.h"
#include "dotstr.h"
#include "strprintf.h"


static void help(const char *program) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> <command> [target node] [parameter] \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nSends a command to Thread <n> of CPU Core <m> of the DataMaster (DM), requires dot file of DM's schedule.\nThere are global commands, that influence the whole DM, local commands influencing the whole thread\nand block commands, that only affect one queue in the schedule.\n");
  fprintf(stderr, "\nGeneral Options:\n");
  fprintf(stderr, "  -c <cpu-idx>              Select CPU core by index, default is 0\n");
  fprintf(stderr, "  -t <thread-idx>           Select thread inside selected CPU core by index, default is 0\n");
  fprintf(stderr, "  -v                        Verbose operation, print more details\n");
  fprintf(stderr, "  -i command .dot file      Dot file containing commands\n");
  fprintf(stderr, "\nGlobal commands:\n");
  fprintf(stderr, "  status                    Show status of all threads and cores (default)\n");
  fprintf(stderr, "  details                   Show time statistics and detailed information on uptime and recent changes\n");
  fprintf(stderr, "  gathertime <Time / ns>    [NOT YET IMPLEMENTED] Set msg gathering time for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> [NOT YET IMPLEMENTED] Set maximum messages in a packet for priority queue\n");
  fprintf(stderr, "  running                   Show bitfield of all running threads on this CPU core\n");
  fprintf(stderr, "  heap                      Show current scheduler heap\n");
  fprintf(stderr, "  startpattern <pattern>    Request start of selected pattern\n");
  fprintf(stderr, "  stoppattern  <pattern>    Request stop of selected pattern\n");
  fprintf(stderr, "  abortpattern <pattern>    Try to immediately abort selected pattern\n");  
  fprintf(stderr, "  chkrem       <pattern>    Check if removal of selected pattern would be safe\n");
  fprintf(stderr, "\nLocal commands:\n");
  fprintf(stderr, "  starttime <Time / ns>     Set start time for this thread\n");
  fprintf(stderr, "  preptime <Time / ns>      Set preparation time (lead) for this thread\n");
  fprintf(stderr, "  deadline                  Show next deadline for this thread\n");
  fprintf(stderr, "  origin <target node>      Set the node with which selected thread will start\n");
  fprintf(stderr, "  origin                    Return the node with which selected thread will start\n");
  fprintf(stderr, "  hex <target node>         Show hex dump of selected Node \n");
  fprintf(stderr, "  start                     Request start of selected thread. Requires a valid origin.\n");
  fprintf(stderr, "  stop                      Request stop of selected thread\n");
  fprintf(stderr, "  abort                     Immediately aborts selected thread\n");
  fprintf(stderr, "  cursor                    Show name of currently active node of selected thread\n");
  fprintf(stderr, "  force                     Force cursor to match origin\n");
  
  fprintf(stderr, "\nBlock commands:\n");
  fprintf(stderr, "  noop <target node>                        [Options: lpq]   Placeholder to stall succeeding commands, has no effect itself\n");
  fprintf(stderr, "  flow <target node> <destination node>     [Options: lpqs]  Changes schedule flow to <Destination Node>\n");
  fprintf(stderr, "  relwait <target node> <wait time / ns>    [Options: lps]   Changes Block period to <wait time>\n");
  fprintf(stderr, "  abswait <target node> <wait time / ns>    [Options: lp]    Stretches Block period until <wait time>\n");
  fprintf(stderr, "  flush <target node> <target priorities>   [Options: lp]    [NOT TESTED] Flushes all pending commands (hex 0x0 - 0x7) of lower priority\n");
  fprintf(stderr, "  queue <target node>                       [Options: p]     Show all queue content (unitialised cmd slots will show garbage) \n");
  fprintf(stderr, "Options for Block commands:\n");
  fprintf(stderr, "  -l <Time / ns>           The absolute time in ns after which the command will become active, default is 0 (immediately)\n");
  fprintf(stderr, "  -p <priority>            The priority of the command (0 = Low, 1 = High, 2 = Interlock), default is 0\n");
  fprintf(stderr, "  -q <quantity>            The number of times the command will be inserted into the target queue, default is 1\n");
  fprintf(stderr, "  -s                       Changes to the schedule are permanent\n");
  fprintf(stderr, "\n");
}


void showStatus(const char *netaddress, CarpeDM& cdm, bool verbose) {
  std::string show;
  uint8_t cpuQty = cdm.getCpuQty();
  uint8_t thrQty = _THR_QTY_;

  std::vector<std::string> vsCursor;
  std::vector<std::string> vsCursorPattern;
  std::vector<std::string> vsOrigin;
  std::vector<std::string> vsOriginPattern;
  std::vector<uint64_t> vsMsgCnt;

  //do this fast to get a most coherent picture, no output
  for(uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for(uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      vsCursor.push_back(cdm.getThrCursor(cpuIdx, thrIdx));
      vsMsgCnt.push_back(cdm.getThrMsgCnt(cpuIdx, thrIdx));
    } 
  }  

  for(uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for(uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      vsCursorPattern.push_back(cdm.getNodePattern(vsCursor[cpuIdx * thrQty + thrIdx]));
      vsOrigin.push_back(cdm.getThrOrigin(cpuIdx, thrIdx));
      vsOriginPattern.push_back(cdm.getNodePattern(vsOrigin[cpuIdx * thrQty + thrIdx]));
    } 
  }


  const uint16_t width = 149;
  //this is horrible code, but harmless. Does the job for now.
  //TODO: replace this with something more sensible

  char date[40];
  uint64_t timeWr = cdm.getDmWrTime();
  uint64_t timeWrNs = timeWr * 1000000000ULL;
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeWr));


  printf("\n\u2552"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2555\n");
  printf("\u2502 DataMaster: %-80s \u2502 WR-Time: 0x%08x%08x ns \u2502 %.19s \u2502\n", netaddress, (uint32_t)(timeWrNs>>32), (uint32_t)timeWrNs, date);
  printf("\u251C"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2524\n");
  printf("\u2502 %3s \u2502 %3s \u2502 %7s \u2502 %9s \u2502 %55s \u2502 %55s \u2502\n", "Cpu", "Thr", "Running", "MsgCount", "Pattern", "Node");
  printf("\u251C"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2524\n");
  
  bool toggle=false;


  for(uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for(uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      if (verbose || ((cdm.getThrRun(cpuIdx) >> thrIdx) & 1)) {
        //if (!first) {printf("\u251C"); for(int i=0;i<width;i++) printf("\u2500"); printf("\u2524\n");
        std::string running = (((cdm.getThrRun(cpuIdx) >> thrIdx) & 1) ? std::string(KGRN) + std::string("yes") : std::string(KRED) + std::string(" no")) + std::string(KNRM);
        std::string originPattern = vsOriginPattern[cpuIdx * thrQty + thrIdx];
        std::string origin        = vsOrigin[cpuIdx * thrQty + thrIdx];

        printf("\u2502%s %2u  \u2502 %2u  \u2502   %3s%s   \u2502 %9llu \u2502 %55s \u2502 %55s %s\u2502\n", (toggle ? BLGR : ""), cpuIdx, thrIdx, running.c_str(), 
          (toggle ? BLGR : ""),
          (unsigned long long int)vsMsgCnt[cpuIdx * thrQty + thrIdx],
          vsCursorPattern[cpuIdx * thrQty + thrIdx].c_str(),  
          vsCursor[cpuIdx * thrQty + thrIdx].c_str(),
          BNRM
        );
        toggle = !toggle;
      }
    } 
  }

  printf("\u2514"); for(int i=0;i<width;i++) printf("\u2500"); printf("\u2518\n");

}

void showHealth(const char *netaddress, CarpeDM& cdm, bool verbose) {
  std::string show;
  uint8_t cpuQty = cdm.getCpuQty();

  HealthReport *hr = new HealthReport[cpuQty];


  for(uint8_t i=0; i < cpuQty; i++) { cdm.getHealth(i, hr[i]); }  
  const uint16_t width = 160;
  //this is horrible code, but harmless. Does the job for now.
  //TODO: replace this with something more sensible


  char date[40];
  uint64_t timeWr = cdm.getDmWrTime();
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeWr));

  
  

  printf("\n\u2552"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2555\n");
  printf("\u2502 DataMaster: %-83s \u2502 WR-Time: 0x%08x%08x \u2502 %.19s \u2502\n", netaddress, (uint32_t)(timeWr>>32), (uint32_t)timeWr, date);
  printf("\u251C"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2524\n");
  printf("\u2502 %3s \u2502 %24s \u2502 %24s \u2502 %8s \u2502 %14s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %10s \u2502\n", 
        "Cpu", "BootTime", "Schedule ModTime", "Issuer", "CPU Msg Cnt", "Min dT", "Max dT", "Avg dT", "Thrs dT", "WrnCnt", "State");
  printf("\u251C"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2524\n");
  

   
  for(uint8_t i=0; i < cpuQty; i++) {
    char dateBoot[40], dateMod[40];
    uint64_t timeBoot = hr[i].bootTime / 1000000000ULL, timeMod = hr[i].smodTime / 1000000000ULL;
    strftime(dateBoot,  sizeof(dateBoot), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeBoot));   
    strftime(dateMod,   sizeof(dateMod),  "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeMod));
    //this is nanoseconds, we need to convert to seconds
    
    
    

    printf("\u2502 %3u \u2502 %.24s \u2502 %.24s \u2502 %8s \u2502 %14llu \u2502 %9d \u2502 %9d \u2502 %9d \u2502 %9d \u2502 %9u \u2502 0x%08x \u2502\n", 
                                                                                                                    hr[i].cpu,
                                                                                                                    dateBoot,
                                                                                                                    dateMod,
                                                                                                                    hr[i].smodIssuer,
                                                                                                                    (unsigned long long int)hr[i].msgCnt,
                                                                                                                    (int)hr[i].minTimeDiff,
                                                                                                                    (int)hr[i].maxTimeDiff,
                                                                                                                    (int)hr[i].avgTimeDiff,
                                                                                                                    (int)hr[i].warningThreshold,
                                                                                                                    hr[i].warningCnt,
                                                                                                                    hr[i].stat);
  }  
  


  printf("\u2514"); for(int i=0;i<width;i++) printf("\u2500"); printf("\u2518\n");

}





int main(int argc, char* argv[]) {



  bool verbose = false, permanent = false;

  int opt;
  const char *program = argv[0];
  const char cTypeName[] = "status"; 
  const char *netaddress, *targetName = NULL, *cmdFilename = NULL, *typeName = (char*)&cTypeName, *para = NULL;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0, cmdPrio = PRIO_LO, cmdQty = 1;
  uint64_t cmdTvalid = 0, longtmp;

// start getopt 
   while ((opt = getopt(argc, argv, "shvc:p:l:t:q:i:")) != -1) {
      switch (opt) {
          case 'i':
            cmdFilename = optarg;
            break;
         case 'v':
            verbose = 1;
            break;
         case 't':
            tmp = strtol(optarg, NULL, 0);
            if ((tmp < 0) || (tmp >= 8)) {
              std::cerr << program << ": Thread idx '" << optarg << "' is invalid. Choose an index between 0 and " << _THR_QTY_ -1 << std::endl;
              error = -1;
            } else {thrIdx = (uint32_t)tmp;}
         case 'l':
            longtmp = strtoll(optarg, NULL, 0);
            if (longtmp < 0) {
              std::cerr << program << ": Valid time must be a positive offset of nanoseconds to UTC 0 (12:00 Jan 1st 1970)" << std::endl;
              error = -1;
            } else {cmdTvalid = (uint64_t)longtmp;}
            break;       
         case 'p':
             tmp = strtol(optarg, NULL, 0);
            if ((tmp < PRIO_LO) || (tmp > PRIO_IL)) {
              std::cerr << program << ": Priority must be 0 (Low), 1 (High) or Interlock (2)  -- '" << std::endl;
               error = -1;
            } else {cmdPrio = (uint32_t)tmp;}

             break;
         case 'q':
            tmp = strtol(optarg, NULL, 0);
            if ((tmp < 1) || (tmp > ACT_QTY_MSK)) {
              std::cerr << program << ": Command quantity must be between 1 and " << ACT_QTY_MSK << std::endl;
              error = -1;
            } else {cmdQty = (uint32_t)tmp;}
            break; 
         case 'c':
            tmp = strtol(optarg, NULL, 0);
            if (tmp < 0) {
              std::cerr << program << ": CPU idx '" << optarg << "' is invalid, must be a positive number " << std::endl;
              error = -1;
            } else {cpuIdx = (uint32_t)tmp;}
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
    if (optind+2 < argc) targetName      = argv[optind+2];
    if (optind+3 < argc) para            = argv[optind+3];
   
  CarpeDM cdm = CarpeDM();

  if(verbose) cdm.verboseOn();

  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
    std::cerr << program << ": Could not connect to DM. Cause: " << err.what() << std::endl; return -20;
  }


  if (!(cdm.isCpuIdxValid(cpuIdx))) {
    std::cerr << program << ": CPU Idx " << cpuIdx << " does not refer to a CPU with valid firmware." << std::endl << std::endl;
    cdm.showCpuList();
    return -30;
  }

  namespace dnt = DotStr::Node::TypeVal;

  uint32_t globalStatus = cdm.getStatus(0), status = cdm.getStatus(cpuIdx);

  if ( !(globalStatus & (SHCTL_STATUS_EBM_INIT_SMSK | SHCTL_STATUS_PQ_INIT_SMSK))
    || !(status & (SHCTL_STATUS_UART_INIT_SMSK | SHCTL_STATUS_DM_INIT_SMSK)) ) 
    {
    std::cerr << program << ": DM is not fully initialised. Cause: " << std::endl;
    if (!(globalStatus & SHCTL_STATUS_EBM_INIT_SMSK)) std::cerr << "EB Master could not be configured. Does the DM have a valid IP?" << std::endl;
    if (!(globalStatus & SHCTL_STATUS_PQ_INIT_SMSK))  std::cerr << "Priority Queue could not be configured" << std::endl;
    if (!(status & SHCTL_STATUS_UART_INIT_SMSK))      std::cerr << "CPU " << cpuIdx << "'s UART is not functional" << std::endl;
    if (!(status & SHCTL_STATUS_DM_INIT_SMSK))      std::cerr << "CPU " << cpuIdx << " could not be initialised" << std::endl;
    //if (!(status & SHCTL_STATUS_WR_INIT_SMSK))      std::cerr << "CPU " << cpuIdx << "'s WR time could not be initialised" << std::endl;
    return -40;
  }

  try { cdm.loadHashDictFile("dm.dict"); } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Warning - Could not load dictionary file. Cause: " << err.what() << std::endl;
    }
  try { cdm.loadGroupsDictFile("dm.groups"); } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Warning - Could not load groups file. Cause: " << err.what() << std::endl;
    }


 
    
  try { 
    cdm.download();
    if(verbose) cdm.showDown(false);
  } catch (std::runtime_error const& err) {
    std::cerr << program << ": Download from CPU "<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
    return -7;
  }
 
  vAdr cmdAdrs;
  vBuf cmdData;
  mc_ptr mc = NULL;

  //check if we got a dot full of commands and send if so
  if (cmdFilename != NULL) {
    try {
          cdm.sendCommandsDotFile(cmdFilename);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not send command .dot " << cmdFilename << ". Cause: " << err.what() << std::endl;
        } 
    return 0;
      
  }


  if (typeName != NULL ) {  

    if(verbose) std::cout << "Generating " << typeName << " command" << std::endl;

    std::string cmp(typeName);

    if      (cmp == dnt::sCmdNoop)  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      mc = (mc_ptr) new MiniNoop(cmdTvalid, cmdPrio, cmdQty );
    }
    else if (cmp == "status")  {
      showStatus(netaddress, cdm, verbose);
      return 0;
    }
    else if (cmp == "details")  {
      showHealth(netaddress, cdm, verbose);
      return 0;
    }   
    else if (cmp == dnt::sCmdFlow)  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if ((para != NULL) && ((para == DotStr::Node::Special::sIdle ) || cdm.isInHashDict( para))) { 
        uint32_t adr; 
        try {
          adr = cdm.getNodeAdr(para, TransferDir::DOWNLOAD, AdrType::INT);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not obtain address of destination node " << para << ". Cause: " << err.what() << std::endl;
        } 
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, permanent );
      } else {std::cerr << program << ": Destination Node '" << para << "'' was not found on DM" << std::endl; return -1; }
    }
    else if (cmp == "relwait")  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
      mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, strtoll(para, NULL, 0), permanent, false );
    }
    else if (cmp == "abswait")  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
        mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, strtoll(para, NULL, 0), permanent, true ); 
    }
    else if (cmp == dnt::sCmdFlush) {
        if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        if (para == NULL) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }  
        uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
        std::cout << "qprio " << para << " 0x" << std::hex << queuePrio << std::endl;
        mc = (mc_ptr) new MiniFlush(cmdTvalid, cmdPrio, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1));
    }
    else if (cmp == "queue") {
        if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        cdm.dumpQueue(cpuIdx, targetName, cmdPrio);
        return 0;
    } 
    else if (cmp == dnt::sCmdOrigin)  {
      if( targetName != NULL) {
        if(!(cdm.isInHashDict( targetName)) && targetName != DotStr::Node::Special::sIdle) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        cdm.setThrOrigin(cpuIdx, thrIdx, targetName);
      }
      if( verbose | (targetName == NULL) ) { std::cout << "CPU " << cpuIdx << " Thr " << thrIdx << " origin points to node " << cdm.getThrOrigin(cpuIdx, thrIdx) << std::endl;}
      return 0;
    }
    else if (cmp == "cursor")  {
      std::cout << "Currently at " << cdm.getThrCursor(cpuIdx, thrIdx) << std::endl;
      return 0;
    }
    else if (cmp == "force")  {
      cdm.forceThrCursor(cpuIdx, thrIdx);
      return 0;
    }
    else if (cmp == "chkrem")  {
      std::string report;
      bool isSafe = cdm.isSafeToRemove(targetName, report);
      cdm.writeTextFile("debug.dot", report);
      std::cout << std::endl << "Pattern " << targetName << " content removal: " << (isSafe ? "SAFE" : "FORBIDDEN" ) << std::endl;
      return 0;
    }
    else if (cmp == dnt::sCmdStart)  {
      //check if a valid origin was assigned before executing
      std::string origin;
      if( targetName != NULL) {
        uint32_t bits = strtol(targetName, NULL, 0);
        for(int i=0; i < _THR_QTY_; i++) {
          if((bits >> i) & 1) {
            origin = cdm.getThrOrigin(cpuIdx, i);
            if ((origin == DotStr::Node::Special::sIdle) || (origin == DotStr::Misc::sUndefined)) {std::cerr << program << ": Cannot start, origin of CPU " << cpuIdx << "'s thread " << thrIdx << " is not a valid node" << std::endl; return -1;}
         } 
        }
        cdm.setThrStart(cpuIdx, bits & ((1<<_THR_QTY_)-1) );
      } else {
        origin = cdm.getThrOrigin(cpuIdx, thrIdx);
        if ((origin == DotStr::Node::Special::sIdle) || (origin == DotStr::Misc::sUndefined)) {std::cerr << program << ": Cannot start, origin of CPU " << cpuIdx << "'s thread " << thrIdx << " is not a valid node" << std::endl; return -1;}
        cdm.startThr(cpuIdx, thrIdx);
      }
      return 0;
    }
    else if (cmp == dnt::sCmdStop)  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      
        uint32_t adr; 
        try {
          adr = cdm.getNodeAdr(DotStr::Node::Special::sIdle , TransferDir::DOWNLOAD, AdrType::INT);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not obtain address of destination node " << para << ". Cause: " << err.what() << std::endl;
        } 
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, permanent );

    }
    else if (cmp == dnt::sCmdAbort)  {
      if( targetName != NULL) {
        uint32_t bits = strtol(targetName, NULL, 0);
       cdm.setThrAbort(cpuIdx, bits & ((1<<_THR_QTY_)-1) );
      } else { cdm.abortThr(cpuIdx, thrIdx); }
      return 0;
    }
    else if (cmp == "startpattern")  {
      //check if a valid origin was assigned before executing
      if( targetName != NULL) {
        cdm.startPattern(targetName, thrIdx );
      } else { std::cout << "Missing valid Pattern name" << std::endl; }
      return 0;
    }
    else if (cmp == "stoppattern")  {
      if( targetName != NULL) {
        cdm.stopPattern(targetName);
      } else { std::cout << "Missing valid Pattern name" << std::endl; }
      return 0;

    }
    else if (cmp == "abortpattern")  {
      if( targetName != NULL) {
        cdm.abortPattern(targetName); 
      } else { std::cout << "Missing valid Pattern name" << std::endl; }
      return 0;
    }
    else if (cmp == "running")  {
      std::cout << "CPU #" << cpuIdx << " Running Threads: 0x" << cdm.getThrRun(cpuIdx) << std::endl;
      return 0;
    }
    else if (cmp == "hex")  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      try {
        cdm.dumpNode(cpuIdx, targetName);
      } catch (std::runtime_error const& err) {
        std::cerr << program << ": Node not found. Cause: " << err.what() << std::endl; return -21;
      }  
      return 0;
    }
    else if (cmp == "heap")  {
      cdm.inspectHeap(cpuIdx); 
      return 0;
    }
    else if (cmp == "starttime")  {
      if( targetName != NULL) { cdm.setThrStartTime(cpuIdx, thrIdx, strtoll(targetName, NULL, 0)); }
      else { std::cout << "CPU " << cpuIdx << " Thr " << thrIdx << " Starttime " << cdm.getThrStartTime(cpuIdx, thrIdx) << std::endl; }
      return 0;
    }
    else if (cmp == "preptime")  {
      if( targetName != NULL) { cdm.setThrPrepTime(cpuIdx, thrIdx, strtoll(targetName, NULL, 0)); }
      else { std::cout << "CPU " << cpuIdx << " Thr " << thrIdx << " Preptime " << cdm.getThrPrepTime(cpuIdx, thrIdx) << std::endl; }
      return 0;
    }
    else if (cmp == "deadline")  {
      std::cout << "CPU " << cpuIdx << " Thr " << thrIdx << " Deadline " << cdm.getThrDeadline(cpuIdx, thrIdx) << std::endl;
      return 0;
    }

    //all the block commands set mc, so...
    if (mc != NULL) {
      try {
          cdm.sendCommand(targetName, cmdPrio, mc);     
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not send command " << para << ". Cause: " << err.what() << std::endl;
        }  
      return 0; 

    } 


    std::cerr << program << ": " << cmp << " is not a valid command. Type " << program << " -h for help" << std::endl;

  }


  return 0;
}
