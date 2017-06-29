#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"
#include "node.h"
#include "block.h"
#include "memunit.h"
#include "minicommand.h"

static void help(const char *program) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> <.dot file> <command> [target node] [parameter] \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nSends a command to Thread <n> of CPU Core <m> of the DataMaster (DM), requires dot file of DM's schedule.\nThere are global commands, that influence the whole DM, local commands influencing the whole thread\nand block commands, that only affect one queue in the schedule.\n");
  fprintf(stderr, "\nGeneral Options:\n");
  fprintf(stderr, "  -c <cpu-idx>              select CPU core by index, default is 0\n");
  fprintf(stderr, "  -t <thread-idx>           select thread inside selected CPU core by index, default is 0\n");
  fprintf(stderr, "  -v                        verbose operation, print more details\n");
  fprintf(stderr, "\nGlobal commands:\n");
  fprintf(stderr, "  gathertime <Time / ns>    [NOT YET IMPLEMENTED] Set msg gathering time for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> [NOT YET IMPLEMENTED] Set maximum messages in a packet for priority queue\n");
  fprintf(stderr, "  clear                     [NOT YET IMPLEMENTED] clear all schedule data on this CPU core\n");
  fprintf(stderr, "  running                   show bitfield of all running threads on this CPU core\n");  
  fprintf(stderr, "\nLocal commands:\n");
  fprintf(stderr, "  preptime <Time / ns>      [NOT YET IMPLEMENTED] Set preparation time (lead) for this thread\n");
  fprintf(stderr, "  origin <target node       Set the node with which selected thread will start\n");
  fprintf(stderr, "  start                     Request start of selected thread. Requires a valid origin.\n");
  fprintf(stderr, "  stop                      Request stop of selected thread\n");
  fprintf(stderr, "  abort                     Immediately aborts selected thread\n");
  fprintf(stderr, "  cursor                    Show name of currently active node of selected thread\n");
  fprintf(stderr, "\nBlock commands:\n");
  fprintf(stderr, "  noop <target node>                        [Options: lpq]   Placeholder to stall succeeding commands, has no effect itself\n");
  fprintf(stderr, "  flow <target node> <destination node>     [Options: lpqs]  Changes schedule flow to <Destination Node>\n");
  fprintf(stderr, "  relwait <target node> <wait time / ns>    [Options: lps]   Changes Block period to <wait time>\n");
  fprintf(stderr, "  abswait <target node> <wait time / ns>    [Options: lp]    [NOT YET IMPLEMENTED] Stretches Block period until <wait time>\n");
  fprintf(stderr, "  flush <target node> <target priorities>   [Options: lp]    [NOT YET IMPLEMENTED] Flushes all pending commands (hex 0x0 - 0x7) of lower priority\n");
  fprintf(stderr, "Options for Block commands:\n");
  fprintf(stderr, "  -l <Time / ns>           the absolute time in ns after which the command will become active, default is 0 (immediately)\n");
  fprintf(stderr, "  -p <priority>            the priority of the command (0 = Low, 1 = High, 2 = Interlock), default is 0\n");
  fprintf(stderr, "  -q <quantity>            the number of times the command will be inserted into the target queue, default is 1\n");
  fprintf(stderr, "  -s                       [NOT YET IMPLEMENTED] Changes to the schedule are permanent\n");
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {



  bool verbose = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *targetName = NULL, *inputFilename = NULL, *typeName = NULL, *para = NULL;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0, cmdPrio = PRIO_LO, cmdQty = 1;
  uint64_t cmdTvalid = 0, cmdFlush = PRIO_LO, longtmp;

// start getopt 
   while ((opt = getopt(argc, argv, "hvc:p:l:t:q:")) != -1) {
      switch (opt) {
         case 'v':
            verbose = 1;
            break;
         case 't':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid thread idx -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {thrIdx = (uint32_t)tmp;}
         case 'l':
            longtmp = atoll(optarg);
            if (longtmp < 0) {
              std::cerr << program << ": invalid valid time -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cmdTvalid = (uint64_t)tmp;}
            break;       
         case 'p':
             tmp = atol(optarg);
            if (tmp < 0 || tmp > 2) {
              std::cerr << program << ": invalid priority -- '" << optarg << "'" << std::endl;
               error = -1;
            } else {cmdPrio = (uint32_t)tmp;}

             break;
         case 'q':
            tmp = atol(optarg);
            if (tmp < 1) {
              std::cerr << program << ": invalid qty -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cmdQty = (uint32_t)tmp;}
            break; 
         case 'c':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid cpu idx -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cpuIdx = (uint32_t)tmp;}
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
 
   if (optind+2 >= argc) {
   std::cerr << program << ": expecting three non-optional argument: <etherbone-device> <.dot file> <command> " << std::endl;
    //help();
    return -4;
    }
    
    // process command arguments
   
    netaddress = argv[optind];
    if (optind+1 < argc) inputFilename   = argv[optind+1];
    if (optind+2 < argc) typeName        = argv[optind+2];
    if (optind+3 < argc) targetName      = argv[optind+3];
    if (optind+4 < argc) para            = argv[optind+4];
   
  CarpeDM cdm = CarpeDM();

  if(verbose) cdm.verboseOn();

  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
    std::cerr << "ERROR - Could not connect to DM: " << err.what() << std::endl; return -20;
  }



  try { cdm.addDotToDict(inputFilename); }
  catch (std::runtime_error const& err) {
    std::cerr << "ERROR: No Nodename/Hash dictionary available. Cause: " << err.what() << std::endl; return -30;
  }
    
  try { 
    cdm.downloadAndParse(cpuIdx);
    if(verbose) cdm.showDown(cpuIdx);
  } catch (std::runtime_error const& err) {
    std::cerr << "ERROR: Download from CPU#"<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
    return -7;
  }
 
  vAdr cmdAdrs;
  vBuf cmdData;
  mc_ptr mc = NULL;

  if (typeName != NULL ) {  

    if(verbose) std::cout << "Trying to generate " << typeName << " command" << std::endl;

    std::string cmp(typeName);

    if      (cmp == "noop")  {
      if(!(cdm.isKnown(targetName))) {std::cerr << "ERROR: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
      mc = (mc_ptr) new MiniNoop(cmdTvalid, cmdPrio, cmdQty );
    }
    else if (cmp == "flow")  {
      if(!(cdm.isKnown(targetName))) {std::cerr << "ERROR: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
      if ((para != NULL) && cdm.isKnown(para)) { 
        uint32_t adr; 
        try {
          adr = cdm.getNodeAdr(cpuIdx, para, DOWNLOAD, INTERNAL);
        } catch (std::runtime_error const& err) {
          std::cerr << "ERROR: Could not obtain address of Destination Node " << para << ". Cause: " << err.what() << std::endl;
        } 
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr );
      } else {std::cerr << "ERROR: Destination Node '" << para << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
    }
    else if (cmp == "flush") {
        if(!(cdm.isKnown(targetName))) {std::cerr << "ERROR: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
    }
    else if (cmp == "relwait")  {
      if(!(cdm.isKnown(targetName))) {std::cerr << "ERROR: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
      if (para != NULL) { mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, atoll(para) ); }
    }
    else if (cmp == "origin")  {
      if (targetName != NULL) { 
        cdm.setThrOrigin(cpuIdx, thrIdx, targetName);     
        if(verbose) std::cout << "CPU #" << cpuIdx << " Thr #" << thrIdx << " Origin was set to Node " << cdm.getThrOrigin(cpuIdx, thrIdx) << std::endl;
      }
    }
    else if (cmp == "cursor")  {
      std::cout << "Currently at " << cdm.getThrCursor(cpuIdx, thrIdx) << std::endl;
      
    }
    else if (cmp == "start")  {
      cdm.startThr(cpuIdx, thrIdx); 
      
    }
    else if (cmp == "stop")  {
      cdm.stopThr(cpuIdx, thrIdx);
      
    }
    else if (cmp == "abort")  {
      cdm.abortThr(cpuIdx, thrIdx);
      
    }
    else if (cmp == "running")  {
      std::cout << "CPU #" << cpuIdx << " Running Threads: 0x" << cdm.getThrRun(cpuIdx) << std::endl;
   
    }

    if (mc != NULL) {
      try {
          cdm.sendCmd(cpuIdx, targetName, cmdPrio, mc);     
        } catch (std::runtime_error const& err) {
          std::cerr << "ERROR: Could not send command " << para << ". Cause: " << err.what() << std::endl;
        }  
       

    }  

  }


  return 0;
}
