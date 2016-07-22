#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wait.h>
#include "access.h"
#include "../ftm_common.h"
#include "fancy.h"


#define FILENAME_LEN   256
#define UINT64_STR_LEN 24
#define DUMP_STR_LEN 65536
#define STATUS_BUF_SIZE 65536

const char* program;
uint8_t verbose, readonly, error, show_time;


static void help(void) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> [command]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  gathertime <Time / ns>    Set msg gathering time for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> Set maximum messages in a packet for priority queue\n\n");
  fprintf(stderr, "  -c <core-idx>             select a core by index, -1 selects all\n");
  fprintf(stderr, "  -v                        verbose operation, print more details\n");
  fprintf(stderr, "  -h                        display this help and exit\n");
  fprintf(stderr, "  -t                        show the White Rabbit time\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  status                    (default) report core status\n");
  fprintf(stderr, "  run                       start this core\n");
  fprintf(stderr, "  stop                      request stop on this core\n");
  fprintf(stderr, "  fstop                     force stop on this core\n");
  fprintf(stderr, "  bpset                     set branchpoint. accepts 0..n or 'idle'\n");
  fprintf(stderr, "  idle                      request idle state on this core\n");
  fprintf(stderr, "  swap                      swap active and inactive page on this core\n");
  fprintf(stderr, "  put <filename>            puts ftm data from xml file to inactive page on this core\n");
  fprintf(stderr, "  sig <offset><value><mask> Writes to FTM's shared memory area for this core. Mask is optional, default is 64b\n");
  fprintf(stderr, "  clear                     clears all pages on this core\n");
  fprintf(stderr, "  get                       gets ftm data from inactive page and displays it\n");
  fprintf(stderr, "  dump                      gets ftm data from active page and displays it\n");
  fprintf(stderr, "  loadfw <filename>         puts firmware from bin file to core\n");
  fprintf(stderr, "  condump                   debug feature. make this core output its data to console\n");
  fprintf(stderr, "  preptime <Time / ns>      Set preparation time on this core\n");
  fprintf(stderr, "\n");

  //fprintf(stderr, "  condget                   debug feature. if this core is wating for a condition value, ask what it is\n");
  //fprintf(stderr, "  condset                   debug feature. set condition value in shared mem for this core\n");
  fprintf(stderr, "\n");
}

int main(int argc, char** argv) {

   //vars for command line args
   program  = argv[0];
   
   int opt;
   char *value_end;
   const char* netaddress, *command;
   char     filename[FILENAME_LEN];
   char     bpstr[10];
   uint64_t uint64val = 0;
   
   uint64_t sigOffset, sigValue, sigMask = 0;
   
   // cpu access related
   int cpuId = 0;
   uint8_t overrideFWcheck;
   int32_t targetCpus, validCpus, validTargetCpus;
   int64_t validTargetThrs; //validTargetThrs is there for compatibility with future versions of access library
   
   overrideFWcheck = 0;
   error      = 0;
   verbose    = 0;
   readonly   = 1;
   sigOffset  = 0;
   sigValue   = 0;
   sigMask    = 0;
   show_time  = 0;
  
   // start getopt 
   while ((opt = getopt(argc, argv, "c:ovht")) != -1) {
      switch (opt) {
         case 'o':
            overrideFWcheck = 1;
            break;
         case 'v':
            verbose = 1;
            break;
         case 't':
            show_time = 1;
            break;
         case 'c':
            cpuId = strtol(optarg, &value_end, 0);
            if (*value_end || cpuId < -1 ||cpuId > 32) {
              fprintf(stderr, "%s: invalid cpu id -- '%s'\n", program, optarg);
              error = 1;
            }
            break;
         case 'h':
            help();
            return 0;
         case ':':
         case '?':
            error = 1;
            break;
         default:
            fprintf(stderr, "%s: bad getopt result\n", program);
            return 1;
      }
   }

   if (error) return 1;

   if (optind >= argc) {
   fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
   fprintf(stderr, "\n");
   help();
   return 1;
   }
   
   // process command arguments
   
   netaddress = argv[optind];
   printf("\n");

   
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
   
   if ( (!strcasecmp(command, "sig")) ) { 
      if (optind+2 < argc) {
         sigOffset  = (uint64_t)strtoll(argv[optind+1], NULL, 16);
         sigValue   = (uint64_t)strtoll(argv[optind+2], NULL, 16);
      } 
      else {
         fprintf(stderr, "%s: expecting two non-optional arguments: <Offset> <Value>\n", program);
         return 1;
      }
      if (optind+3 < argc) {
         sigMask    = (uint64_t)strtoll(argv[optind+3], NULL, 16);
      } else { sigMask = 0xffffffffffffffff; }
   } 
   
   if ( (!strcasecmp(command, "preptime")) || (!strcasecmp(command, "gathertime")) || (!strcasecmp(command, "maxmsg"))) { 
      if (optind+1 < argc) {
         long long unsigned int tmp = strtoll(argv[optind+1], NULL, 10);
         uint64val = (uint64_t)tmp;
      } else {
         if(!strcasecmp(command, "maxmsg")) fprintf(stderr, "%s: expecting one non-optional argument: <Message Quantity>\n", program);
         else fprintf(stderr, "%s: expecting one non-optional argument: <Time / ns>\n", program);
         return 1;
      }
   }
   
   if (!strcasecmp(command, "setbp")) {
      if (optind+1 < argc) {
         bpstr[9] = 0;
         strncpy(bpstr, argv[optind+1], 8);
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <branchpoint name>\n", program);
         return 1;
      }
   }
   
//*****************************************************************************************************************//

  if (verbose) printf("ftm-ctl v%s\n", TOOL_VER);
  validCpus = ftmOpen(netaddress, overrideFWcheck);
  
  if(validCpus <= 0) {
    if(validCpus == 0) fprintf(stderr, "ERROR: No valid DM CPUs found.\n");
    return -1;
  } 
  
  if(cpuId < 0) {
    targetCpus = (1 << p->cpuQty) -1;
  } else if (cpuId <= p->cpuQty -1) {
    targetCpus = 1 << cpuId;
  } else {
    fprintf(stderr, "%s: invalid cpu id %u. Valid id's are -1 or 0..%u\n", program, cpuId, p->cpuQty -1);
    return -1;
  }  
  
  validTargetCpus = validCpus & targetCpus;
  validTargetThrs = cpus2thrs(validTargetCpus);
  
  
         
// DM prioQ Operations   
  if(!strcasecmp(command, "gathertime")) {
    return ftmSetDuetime(uint64val);
  }
 
 
  if(!strcasecmp(command, "maxmsg")) {
    return ftmSetMaxMsgs(uint64val);
  }
 
//DM CPU Operations
  if (!strcasecmp(command, "loadfw")) {
    return ftmFwLoad(targetCpus, filename); // all selected, not just the ones with valid firmware !
  }      
        
  /* -------------------------------------------------------------------- */
   if (!strcasecmp(command, "status")) {
  
    uint32_t* stateBuff = (uint32_t*)malloc(STATUS_BUF_SIZE);
    ftmFetchStatus(stateBuff, STATUS_BUF_SIZE);
    printf("%s### FTM @ %s - %u Cores ####%s\n", KCYN, netaddress, p->cpuQty, KNRM);
    ftmShowStatus(targetCpus, stateBuff, verbose);
    free(stateBuff);
  }

  /* -------------------------------------------------------------------- */
  else if (!strcasecmp(command, "run")) {
    ftmCommand(validTargetThrs, CMD_START);
  }

  else if (!strcasecmp(command, "stop")) {
     ftmCommand(validTargetThrs, CMD_STOP_REQ);
  }

  else if (!strcasecmp(command, "idle")) {
   ftmCommand(validTargetThrs, CMD_IDLE);
  }
  else if (!strcasecmp(command, "fstop")) {
   ftmCommand(validTargetThrs, CMD_STOP_NOW); 
  } 

  else if (!strcasecmp(command, "swap")) {
   ftmCommand(validTargetThrs, CMD_COMMIT_PAGE);
  } 

  else if (!strcasecmp(command, "condump")) {
   ftmCommand(validTargetThrs, CMD_SHOW_ACT);
  } 
  
  else if (!strcasecmp(command, "clear")) {
   ftmClear(validTargetThrs);
  }
  
  else if (!strcasecmp(command, "reset")) {
     ftmCpuRst(targetCpus);
  }

  else if (!strcasecmp(command, "preptime")) {
     ftmSetPreptime(validTargetThrs, uint64val);
  }
  
  else if(!strcasecmp(command, "put")) {
    if(!readonly) {
      uint32_t res;
      res = ftmPutFile(validTargetThrs, filename);
#if MOCKUP==1
      char* pBufDump = (char*)malloc(DUMP_STR_LEN); 
      ftmDump(validTargetThrs, BUF_SIZE, INACTIVE, pBufDump, DUMP_STR_LEN);
      printf("%s\n", pBufDump);
      free(pBufDump);
      res = 0;
#endif
      return res;   
    } else { fprintf(stderr, "No xml file specified\n"); return -1;}
  }
  
  else if(!strcasecmp(command, "dump")) {
    char* pBufDump = (char*)malloc(DUMP_STR_LEN); 
    ftmDump(validTargetThrs, BUF_SIZE, ACTIVE, pBufDump, DUMP_STR_LEN);
    printf("%s\n", pBufDump);
    free(pBufDump);
    return 0;
  }   
  
  else if(!strcasecmp(command, "get")) {
    char* pBufDump = (char*)malloc(DUMP_STR_LEN); 
    ftmDump(validTargetThrs, BUF_SIZE, INACTIVE, pBufDump, DUMP_STR_LEN);
    printf("%s\n", pBufDump);
    free(pBufDump);
    return 0; 
  }

  else if (!strcasecmp(command, "setbp")) {
    int planIdx;
    if(!strcasecmp(bpstr, "idle")) planIdx = -1;
    else {planIdx = strtol(bpstr, 0, 10);}  	
    return ftmSetBp(validTargetThrs, planIdx);
  }
  
  else if (!strcasecmp(command, "sig")) {
    return ftmSignal(validTargetThrs, sigOffset, sigValue, sigMask);  
  }
     
  else  printf("Unknown command: %s\n", command);  

#if MOCKUP==1
  uint32_t* stateBuff = (uint32_t*)malloc(STATUS_BUF_SIZE);
  ftmFetchStatus(stateBuff, STATUS_BUF_SIZE/4);
  printf("%s### FTM @ %s ####%s\n", KCYN, netaddress, KNRM);
  ftmShowStatus(targetCpus, stateBuff, 1);
  free(stateBuff);
#endif  


  return 0;
}


