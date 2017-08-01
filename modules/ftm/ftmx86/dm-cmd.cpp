#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"
#include "node.h"
#include "block.h"
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
  fprintf(stderr, "  origin <target node>      Set the node with which selected thread will start\n");
  fprintf(stderr, "  origin                    Return the node with which selected thread will start\n");
  fprintf(stderr, "  hex <target node>         Show hex dump of selected Node \n");
  fprintf(stderr, "  start                     Request start of selected thread. Requires a valid origin.\n");
  fprintf(stderr, "  stop                      Request stop of selected thread\n");
  fprintf(stderr, "  abort                     Immediately aborts selected thread\n");
  fprintf(stderr, "  cursor                    Show name of currently active node of selected thread\n");
  fprintf(stderr, "\nBlock commands:\n");
  fprintf(stderr, "  noop <target node>                        [Options: lpq]   Placeholder to stall succeeding commands, has no effect itself\n");
  fprintf(stderr, "  flow <target node> <destination node>     [Options: lpqs]  Changes schedule flow to <Destination Node>\n");
  fprintf(stderr, "  relwait <target node> <wait time / ns>    [Options: lps]   Changes Block period to <wait time>\n");
  fprintf(stderr, "  abswait <target node> <wait time / ns>    [Options: lp]    Stretches Block period until <wait time>\n");
  fprintf(stderr, "  flush <target node> <target priorities>   [Options: lp]    [NOT TESTED] Flushes all pending commands (hex 0x0 - 0x7) of lower priority\n");
  fprintf(stderr, "  queue <target node>                       [Options: p]     Show all queue content (unitialised cmd slots will show garbage) \n");
  fprintf(stderr, "Options for Block commands:\n");
  fprintf(stderr, "  -l <Time / ns>           the absolute time in ns after which the command will become active, default is 0 (immediately)\n");
  fprintf(stderr, "  -p <priority>            the priority of the command (0 = Low, 1 = High, 2 = Interlock), default is 0\n");
  fprintf(stderr, "  -q <quantity>            the number of times the command will be inserted into the target queue, default is 1\n");
  fprintf(stderr, "  -s                       [NOT YET IMPLEMENTED] Changes to the schedule are permanent\n");
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {



  bool verbose = false, permanent = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *targetName = NULL, *inputFilename = NULL, *typeName = NULL, *para = NULL;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0, cmdPrio = PRIO_LO, cmdQty = 1;
  uint64_t cmdTvalid = 0, longtmp;

// start getopt 
   while ((opt = getopt(argc, argv, "shvc:p:l:t:q:")) != -1) {
      switch (opt) {
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
    std::cerr << program << ": Could not connect to DM. Cause: " << err.what() << std::endl; return -20;
  }


  if (!(cdm.isCpuIdxValid(cpuIdx))) {
    std::cerr << program << ": CPU Idx " << cpuIdx << " does not refer to a CPU with valid firmware." << std::endl << std::endl;
    cdm.showCpuList();
    return -30;
  }



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

  cdm.getHashMap().load("dm.dict");

  try { cdm.addDotToDict(inputFilename); }
  catch (std::runtime_error const& err) {
    std::cerr << program << ": Could not insert your .dot file into dictionary. Cause: " << err.what() << std::endl; return -30;
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

  if (typeName != NULL ) {  

    if(verbose) std::cout << "Generating " << typeName << " command" << std::endl;

    std::string cmp(typeName);

    if      (cmp == "noop")  {
      if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      mc = (mc_ptr) new MiniNoop(cmdTvalid, cmdPrio, cmdQty );
    }
    else if (cmp == "flow")  {
      if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if ((para != NULL) && cdm.isValid( para)) { 
        uint32_t adr; 
        try {
          adr = cdm.getNodeAdr(para, DOWNLOAD, INTERNAL);
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not obtain address of destination node " << para << ". Cause: " << err.what() << std::endl;
        } 
        mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, adr, permanent );
      } else {std::cerr << program << ": Destination Node '" << para << "'' was not found on DM" << std::endl; return -1; }
    }
    else if (cmp == "relwait")  {
      if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
      mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, strtoll(para, NULL, 0), permanent, false );
    }
    else if (cmp == "abswait")  {
      if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
      if (para == NULL) {std::cerr << program << ": Wait time in ns is missing" << std::endl; return -1; }
        mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, strtoll(para, NULL, 0), permanent, true ); 
    }
    else if (cmp == "flush") {
        if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        if (para == NULL) {std::cerr << program << ": Queues to be flushed are missing, require 3 bit as hex (IL HI LO 0x0 - 0x7)" << std::endl; return -1; }  
        uint32_t queuePrio = strtol(para, NULL, 0) & 0x7;
        std::cout << "qprio " << para << " 0x" << std::hex << queuePrio << std::endl;
        mc = (mc_ptr) new MiniFlush(cmdTvalid, cmdPrio, (bool)(queuePrio >> PRIO_IL & 1), (bool)(queuePrio >> PRIO_HI & 1), (bool)(queuePrio >> PRIO_LO & 1));
    }
    else if (cmp == "queue") {
        if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        cdm.dumpQueue(cpuIdx, targetName, cmdPrio);
        return 0;
    } 
    else if (cmp == "origin")  {
      if( targetName != NULL) {
        if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
        cdm.setThrOrigin(cpuIdx, thrIdx, targetName);
      }
      if( verbose | (targetName == NULL) ) { std::cout << "CPU " << cpuIdx << " Thr " << thrIdx << " origin points to node " << cdm.getThrOrigin(cpuIdx, thrIdx) << std::endl;}
      return 0;
    }
    else if (cmp == "cursor")  {
      std::cout << "Currently at " << cdm.getThrCursor(cpuIdx, thrIdx) << std::endl;
      return 0;
    }
    else if (cmp == "start")  {
      //check if a valid origin was assigned before executing
      std::string origin;
      if( targetName != NULL) {
        uint32_t bits = strtol(targetName, NULL, 0);
        for(int i=0; i < _THR_QTY_; i++) {
          if((bits >> i) & 1) {
            origin = cdm.getThrOrigin(cpuIdx, i);
            if ((origin == "Idle") || (origin == "Unknown")) {std::cerr << program << ": Cannot start, origin of CPU " << cpuIdx << "'s thread " << thrIdx << " is not a valid node" << std::endl; return -1;}
         } 
        }
        cdm.setThrStart(cpuIdx, bits & ((1<<_THR_QTY_)-1) );
      } else {
        origin = cdm.getThrOrigin(cpuIdx, thrIdx);
        if ((origin == "Idle") || (origin == "Unknown")) {std::cerr << program << ": Cannot start, origin of CPU " << cpuIdx << "'s thread " << thrIdx << " is not a valid node" << std::endl; return -1;}
        cdm.startThr(cpuIdx, thrIdx);
      }
      return 0;
    }
    else if (cmp == "stop")  {
      if( targetName != NULL) { 
        uint32_t bits = strtol(targetName, NULL, 0);
        cdm.setThrStop(cpuIdx, bits & ((1<<_THR_QTY_)-1) ); 
      } else { cdm.stopThr(cpuIdx, thrIdx); }
      return 0;
    }
    else if (cmp == "abort")  {
      if( targetName != NULL) {
        uint32_t bits = strtol(targetName, NULL, 0);
       cdm.clrThrRun(cpuIdx, bits & ((1<<_THR_QTY_)-1) );
      } else { cdm.abortThr(cpuIdx, thrIdx); }
      return 0;
    }
    else if (cmp == "running")  {
      std::cout << "CPU #" << cpuIdx << " Running Threads: 0x" << cdm.getThrRun(cpuIdx) << std::endl;
      return 0;
    }
    else if (cmp == "hex")  {
      if(!(cdm.isValid( targetName))) {std::cerr << program << ": Target node '" << targetName << "'' was not found on DM" << std::endl; return -1; }
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
          cdm.sendCmd(targetName, cmdPrio, mc);     
        } catch (std::runtime_error const& err) {
          std::cerr << program << ": Could not send command " << para << ". Cause: " << err.what() << std::endl;
        }  
      return 0; 

    } 


    std::cerr << program << ": " << cmp << " is not a valid command. Type " << program << " -h for help" << std::endl;

  }


  return 0;
}
