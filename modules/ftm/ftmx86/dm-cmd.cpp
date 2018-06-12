#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>


#include "carpeDM.h"
#include "node.h"
#include "block.h"
#include "minicommand.h"
#include "dotstr.h"
#include "strprintf.h"
#include "filenames.h"

namespace dnt = DotStr::Node::TypeVal;


static void help(const char *program) {
  fprintf(stderr, "\ndm-cmd v%s, build date %s\nSends a command or dotfile of commands to the DM\nThere are global, local and queued commands\n", TOOL_VER, BUILD_DATE);
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> <command> [target node] [parameter] \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nGeneral Options:\n");
  fprintf(stderr, "  -c <cpu-idx>              Select CPU core by index, default is 0\n");
  fprintf(stderr, "  -t <thread-idx>           Select thread inside selected CPU core by index, default is 0\n");
  fprintf(stderr, "  -v                        Verbose operation, print more details\n");
  fprintf(stderr, "  -d                        Debug operation, print everything\n");
  fprintf(stderr, "  -i command .dot file      Run commands from dot file\n");
  fprintf(stderr, "  status                    Show status of all threads and cores (default)\n");
  fprintf(stderr, "  details                   Show time statistics and detailed information on uptime and recent changes\n");
  fprintf(stderr, "  gathertime <Time / ns>    [NOT YET IMPLEMENTED] Set msg gathering time for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> [NOT YET IMPLEMENTED] Set maximum messages in a packet for priority queue\n");
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
  fprintf(stderr, "  start                     Request start of selected thread. Requires a valid origin.\n");
  fprintf(stderr, "  stop                      Request stop of selected thread. Does reverse lookup of current pattern, prone to race condition\n");
  //fprintf(stderr, "  cease                   Cease thread at pattern end.\n");
  fprintf(stderr, "  abort                     Immediately abort selected thread.\n");
  fprintf(stderr, "  halt                      Immediately aborts all threads on all cpus\n");
  //fprintf(stderr, "  force                     Force cursor to match origin\n");
  fprintf(stderr, "  staticflush <target> <prios>                  Flushes all pending commands of given priorities (3b Hi-Md-Lo -> 0x0..0x7) in an inactive (static) block of the schedule\n");
  fprintf(stderr, "  staticflushpattern <pattern> <prios>          Flushes all pending commands of given priorities (3b Hi-Md-Lo -> 0x0..0x7) in an inactive (static) pattern of the schedule\n");
  fprintf(stderr, "\nQueued commands (viable options in square brackets):\n");
  fprintf(stderr, "  stop <target>                        [laps]   Request stop at selected block (flow to idle)\n");
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
  fprintf(stderr, "\n");
}


void showStatus(const char *netaddress, CarpeDM& cdm, bool verbose) {
  std::string show;
  cdm.showCpuList();
  if(cdm.isOptimisedS2R()) cdm.dirtyCtShow();
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
  uint64_t timeWrNs = cdm.getDmWrTime();
  uint64_t timeWr = timeWrNs / 1000000000ULL;
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeWr));


  printf("\n\u2554"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2557\n");
  printf("\u2551 DataMaster: %-80s \u2502 WR-Time: 0x%08x%08x ns \u2502 %.19s \u2551\n", netaddress, (uint32_t)(timeWrNs>>32), (uint32_t)timeWrNs, date);
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 %3s \u2502 %3s \u2502 %7s \u2502 %9s \u2502 %55s \u2502 %55s \u2551\n", "Cpu", "Thr", "Running", "MsgCount", "Pattern", "Node");
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  
  bool toggle=false;


  for(uint8_t cpuIdx=0; cpuIdx < cpuQty; cpuIdx++) {
    for(uint8_t thrIdx=0; thrIdx < thrQty; thrIdx++) {
      if (verbose || ((cdm.getThrRun(cpuIdx) >> thrIdx) & 1)) {
        //if (!first) {printf("\u2560"); for(int i=0;i<width;i++) printf("\u2500"); printf("\u2563\n");
        std::string running = (((cdm.getThrRun(cpuIdx) >> thrIdx) & 1) ? std::string(KGRN) + std::string("yes") : std::string(KRED) + std::string(" no")) + std::string(KNRM);
        std::string originPattern = vsOriginPattern[cpuIdx * thrQty + thrIdx];
        std::string origin        = vsOrigin[cpuIdx * thrQty + thrIdx];

        printf("\u2551%s %2u  \u2502 %2u  \u2502   %3s%s   \u2502 %9llu \u2502 %55s \u2502 %55s %s\u2551\n", (toggle ? BLGR : ""), cpuIdx, thrIdx, running.c_str(), 
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

  printf("\u255A"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u255D\n");

}

void showHealth(const char *netaddress, CarpeDM& cdm, bool verbose) {
  std::string show;
  uint8_t cpuQty = cdm.getCpuQty();

  HealthReport *hr = new HealthReport[cpuQty];


  for(uint8_t i=0; i < cpuQty; i++) { cdm.getHealth(i, hr[i]); }  
  const uint16_t width = 80;
  //this is horrible code, but harmless. Does the job for now.
  //TODO: replace this with something more sensible


  char date[40];
  uint64_t timeWr = cdm.getDmWrTime() / 1000000000ULL ;
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeWr));
  unsigned netStrLen;
  for(netStrLen = 0; netStrLen < width; netStrLen++) {if (netaddress[netStrLen] == '\00') break;}

  printf("\n\u2554"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2557\n");
  printf("\u2551 DM: %s", netaddress); for(uint8_t i=0; i < (width - 5 - netStrLen); i++) printf(" "); printf("\u2551\n");
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 WR-Time: 0x%08x%08x \u2502 %.19s %32s\n", (uint32_t)(timeWr>>32), (uint32_t)timeWr, date, "\u2551");

  
  // Boot Time and Msg Count
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 %3s \u2502 %19s \u2502 %14s \u2502 %37s\n", 
      "Cpu", "BootTime", "CPU Msg Cnt", "\u2551");
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for(uint8_t i=0; i < cpuQty; i++) {
    char date[40];
    uint64_t timeval = hr[i].bootTime / 1000000000ULL; //all times are in nanoseconds, we need to convert to seconds
    strftime(date,  sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeval)); //human readable date
    printf("\u2551 %3u \u2502 %.19s \u2502 %14llu \u2502 %37s\n", hr[i].cpu, date, (long long unsigned int)hr[i].msgCnt, "\u2551");    
  }

  // Most recent schedule modification (time, issuer, type of operation)
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 %3s \u2502 %19s \u2502 %8s \u2502 %8s \u2502 %10s \u2502 %19s\n", "Cpu",  "Schedule ModTime", "Issuer", "Host", "Op Type", "\u2551");
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for(uint8_t i=0; i < cpuQty; i++) {
    char date[40];
    uint64_t timeval = hr[i].smodTime / 1000000000ULL;
    strftime(date,  sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeval));
    printf("\u2551 %3u \u2502 %19s \u2502 %8s \u2502 %8s \u2502 %10s \u2502 %19s\n", hr[i].cpu, date, hr[i].smodIssuer, hr[i].smodHost, hr[i].smodOpType.c_str(), "\u2551");
  }  
  
  // Most recent command (time, issuer, type of operation)
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 %3s \u2502 %19s \u2502 %8s \u2502 %8s \u2502 %10s \u2502 %19s\n", "Cpu",  "Command ModTime", "Issuer", "Host", "Op Type", "\u2551");
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for(uint8_t i=0; i < cpuQty; i++) {
    char date[40];
    uint64_t timeval = hr[i].cmodTime / 1000000000ULL;
    strftime(date,  sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&timeval));
    printf("\u2551 %3u \u2502 %19s \u2502 %8s \u2502 %8s \u2502 %10s \u2502 %19s\n", hr[i].cpu, date, hr[i].cmodIssuer, hr[i].cmodHost, hr[i].cmodOpType.c_str(), "\u2551");
  } 

  //LM32 message ispatch statistics (min lead, max lead, avg lead, lead warning threshold, warning count, status register)
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  printf("\u2551 %3s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %9s \u2502 %10s   %3s\n", 
      "Cpu", "Min dT", "Max dT", "Avg dT", "Thrs dT", "WrnCnt", "State", "\u2551");
  printf("\u2560"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u2563\n");
  for(uint8_t i=0; i < cpuQty; i++) {
    printf("\u2551 %3u \u2502 %9d \u2502 %9d \u2502 %9d \u2502 %9d \u2502 %9u \u2502 0x%08x   %3s\n", 
      hr[i].cpu,
      (int)hr[i].minTimeDiff,
      (int)hr[i].maxTimeDiff,
      (int)hr[i].avgTimeDiff,
      (int)hr[i].warningThreshold,
      hr[i].warningCnt,
      hr[i].stat, "\u2551");
  }
    
  printf("\u255A"); for(int i=0;i<width;i++) printf("\u2550"); printf("\u255D\n");

}





int main(int argc, char* argv[]) {



  bool verbose = false, permanent = false, debug=false, vabs=false, force=false;

  int opt;
  const char *program = argv[0];
  const char cTypeName[] = "status"; 
  const char *netaddress, *targetName = NULL, *cmdFilename = NULL, *typeName = (char*)&cTypeName, *para = NULL;
  char dirnameBuff[80];
  const char *dirname = (const char *)getcwd(dirnameBuff, 80); 


  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0, cmdPrio = PRIO_LO, cmdQty = 1;
  uint64_t cmdTvalid = 0, longtmp;

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
  if(debug)   cdm.debugOn();



  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
    std::cerr << program << ": Could not connect to DM. Cause: " << err.what() << std::endl; return -20;
  }

  if (!(cdm.isCpuIdxValid(cpuIdx))) {
    
    if (!(cdm.isCpuIdxValid(cpuIdx))) {
      std::cerr << program << ": CPU Idx " << cpuIdx << " does not refer to a CPU with valid firmware." << std::endl << std::endl;
       return -30;
    }  
  }

  cdm.updateModTime();

  // The global hard abort commands are special - they must work regardless if the schedule download/parse was successful or not
  if (typeName != NULL ) { 

    std::string tmpGlobalCmds(typeName);

    if (tmpGlobalCmds == dnt::sCmdAbort)  {
      if( targetName != NULL) {
        uint32_t bits = strtol(targetName, NULL, 0);
       cdm.setThrAbort(cpuIdx, bits & ((1<<_THR_QTY_)-1) );
      } else { cdm.abortThr(cpuIdx, thrIdx); }
      return 0;
    }
    else if (tmpGlobalCmds == "halt")  {
      cdm.halt();
      return 0;
    }
  }  

 
    
  try {
   cdm.download();
  } catch (std::runtime_error const& err) {
    std::cerr << program << ": Download from CPU "<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
    return -7;
  }
  

  uint32_t globalStatus = cdm.getStatus(0), status = cdm.getStatus(cpuIdx);

  if (!force && ( !(globalStatus & (SHCTL_STATUS_EBM_INIT_SMSK | SHCTL_STATUS_PQ_INIT_SMSK))
    || !(status & (SHCTL_STATUS_UART_INIT_SMSK | SHCTL_STATUS_DM_INIT_SMSK)) )) 
    {
    std::cerr << program << ": DM is not fully initialised. Cause: " << std::endl;
    if (!(globalStatus & SHCTL_STATUS_EBM_INIT_SMSK)) std::cerr << "EB Master could not be configured. Does the DM have a valid IP?" << std::endl;
    if (!(globalStatus & SHCTL_STATUS_PQ_INIT_SMSK))  std::cerr << "Priority Queue could not be configured" << std::endl;
    if (!(status & SHCTL_STATUS_UART_INIT_SMSK))      std::cerr << "CPU " << cpuIdx << "'s UART is not functional" << std::endl;
    if (!(status & SHCTL_STATUS_DM_INIT_SMSK))        std::cerr << "CPU " << cpuIdx << " could not be initialised" << std::endl;
    //if (!(status & SHCTL_STATUS_WR_INIT_SMSK))      std::cerr << "CPU " << cpuIdx << "'s WR time could not be initialised" << std::endl;
    return -40;
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

  uint64_t tvalidOffs = cdm.getModTime();
  if(!vabs) cmdTvalid += tvalidOffs; // already added modTime when !vabs, so when calling cdm.adjustValidTime, we'll always say tvalid is absolute 


  if (typeName != NULL ) {  

    if(verbose) std::cout << "Generating " << typeName << " command" << std::endl;

    std::string cmp(typeName);

    if      (cmp == dnt::sCmdNoop)  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      cdm.adjustValidTime(cmdTvalid, false);
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
    else if (cmp == "flowpattern")  {
      if ((targetName == NULL) || (para == NULL)) {std::cerr << program << ": Need valid target and destination pattern names " << std::endl; return -1; }

      std::string fromNode = cdm.getPatternExitNode(targetName);
      std::string toNode   = (para == DotStr::Node::Special::sIdle ) ? DotStr::Node::Special::sIdle : cdm.getPatternEntryNode(para);
      
      if ( cdm.isInHashDict( fromNode ) && ( (toNode == DotStr::Node::Special::sIdle ) || cdm.isInHashDict( toNode )  )) { 
        uint32_t adr; 
        try {
          adr = cdm.getNodeAdr(toNode, TransferDir::DOWNLOAD, AdrType::INT);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not obtain address of destination node " << toNode << ". Cause: " << err.what() << std::endl;
        }
        cdm.adjustValidTime(cmdTvalid, true);  
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, permanent );
      } else {std::cerr << program << ": Destination Node '" << toNode << "'' was not found on DM" << std::endl; return -1; }
      targetName = fromNode.c_str();
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
        cdm.adjustValidTime(cmdTvalid, true); 
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, permanent );
      } else {std::cerr << program << ": Destination Node '" << para << "'' was not found on DM" << std::endl; return -1; }
    }
    else if (cmp == "relwait")  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
      cdm.adjustValidTime(cmdTvalid, true); 
      mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, strtoll(para, NULL, 0), permanent, false );
    }
    else if (cmp == "abswait")  {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
      cdm.adjustValidTime(cmdTvalid, true); 
      mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, strtoll(para, NULL, 0), permanent, true ); 
    }
    else if (cmp == dnt::sCmdFlush) {
        if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        if (para == NULL) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }  
        uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
        cdm.adjustValidTime(cmdTvalid, true); 
        mc = (mc_ptr) new MiniFlush(cmdTvalid, cmdPrio, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1));
    }
    else if (cmp == "staticflush") {
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }  
      uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
      try {
          cdm.staticFlushBlock(targetName, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1), force);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not statically flush " << targetName << ". Cause: " << err.what() << std::endl;
        }

      return 0;
    }
    else if (cmp == "queue") {
        if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        std::string report;
        std::cout << cdm.inspectQueues(targetName, report) << std::endl;
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
      cdm.writeTextFile(std::string(dirname) + "/" + std::string(debugfile), report);
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
      if (targetName == NULL) {std::cerr << program << ": expected name of target node" << std::endl; return -1; }
      if(!(cdm.isInHashDict( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      
        uint32_t adr; 
        try {
          adr = cdm.getNodeAdr(DotStr::Node::Special::sIdle , TransferDir::DOWNLOAD, AdrType::INT);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not obtain address of destination node " << para << ". Cause: " << err.what() << std::endl;
        } 
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, permanent );

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
    else if (cmp == "staticflushpattern")  {
      if( targetName != NULL) {
        if (para == NULL) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }  
        uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
        cdm.staticFlushPattern(targetName, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1), force); 
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
