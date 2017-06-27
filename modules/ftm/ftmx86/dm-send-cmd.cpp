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



int main(int argc, char* argv[]) {



  bool verbose = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *targetName = NULL, *inputFilename = NULL, *typeName = NULL, *para = NULL;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0, cmdPrio = PRIO_LO, cmdQty = 1;
  uint64_t cmdTvalid = 0, cmdTWait = 0, cmdFlush = PRIO_LO, longtmp;

// start getopt 
   while ((opt = getopt(argc, argv, "vc:p:t:q:")) != -1) {
      switch (opt) {
         case 'v':
            verbose = 1;
            break;
         case 't':
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
         /*    
         case 'h':
            help();
            return 0;
         */ 
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
   std::cerr << program << ": expecting two non-optional argument: <etherbone-device> <command type> " << std::endl;
    //help();
    return -4;
    }
    
    // process command arguments
   
    netaddress = argv[optind];
    if (optind+1 < argc) inputFilename   = argv[optind+1];
    if (optind+2 < argc) typeName        = argv[optind+2];
    if (optind+3 < argc) targetName      = argv[optind+3];
    if (optind+4 < argc) para            = argv[optind+4];
   

   /*
   if (optind+1 < argc)  command = argv[++optind];
   else                 {command = "status"; cpuId = -1;}
   if (!strcasecmp(command, "loadfw")) overrideFWcheck = 1;  
   
   if ( (!strcasecmp(command, "put")) || (!strcasecmp(command, "loadfw")))
   {
      if (optind+1 < argc) {
         strncpy(filename, argv[optind+1], FILENAME_LEN);

         readonly = 0;
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <filename>\n", program);
         return 1;
      }
   } 
   */

  CarpeDM cdm = CarpeDM();

  
  cdm.connect(std::string(netaddress));

  std::cout << "Creating Dictionary from " << inputFilename << " ... ";
  try { cdm.addDotToDict(inputFilename); }
  catch (...) {
    std::cerr << "Failed to create Dictionary." << std::endl;
    return -10;
  }
  std::cout << "Done." << std::endl;



  std::cout << "Downloading from CPU #" << cpuIdx << "... ";
  cdm.downloadAndParse(cpuIdx);
  std::cout << "Done." << std::endl;

  vAdr cmdAdrs;
  vBuf cmdData;
  mc_ptr mc = NULL;

  if (typeName != NULL ) {  

    std::cout << "Trying to generate " << typeName << " command" << std::endl;

    std::string cmp(typeName);

    if      (cmp == "noop")  {
      if(!(cdm.isKnown(targetName))) {std::cerr << "Error: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
      mc = (mc_ptr) new MiniNoop(cmdTvalid, cmdPrio, cmdQty );
    }
    else if (cmp == "flow")  {
      if(!(cdm.isKnown(targetName))) {std::cerr << "Error: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
      if ((para != NULL) && cdm.isKnown(para)) { mc = (mc_ptr) new MiniFlow(cmdTvalid, cmdPrio, cmdQty, cdm.getNodeAdr(cpuIdx, para, DOWNLOAD, INTERNAL) ); }
    }
    else if (cmp == "flush") {
        if(!(cdm.isKnown(targetName))) {std::cerr << "Error: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
    }
    else if (cmp == "wait")  {
      if(!(cdm.isKnown(targetName))) {std::cerr << "Error: Target Node '" << targetName << "'' is not described in " << inputFilename << ", aborting" << std::endl; return -1; }
      if (para != NULL) { mc = (mc_ptr) new MiniWait(cmdTvalid, cmdPrio, atoll(para) ); }
    }
    else if (cmp == "origin")  {
      if (targetName != NULL) { 
        cdm.setThrOrigin(cpuIdx, thrIdx, targetName);     
        std::cout << "Origin Node was set to " << cdm.getThrOrigin(cpuIdx, thrIdx) << std::endl;
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


    if (mc != NULL) {

      cdm.sendCmd(cpuIdx, targetName, cmdPrio, mc);      

    }  

  }


  return 0;
}
