#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <etherbone.h>
#include "ftmx86.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

#define MAX_DEVICES 100

const char* program;



typedef enum { false = 0, true = 1 } bool;

const    uint32_t devID_RAM 	      = 0x66cfeb52;
const    uint64_t vendID_CERN       = 0x000000000000ce42;
char              devName_RAM_pre[] = "WB4-BlockRAM_";		
char              devName_RAM_post[] = "010";


static eb_width_t address_width, data_width;
static int verbose;
static int idx;
int eb_ram_com(const char* netaddress, uint8_t cpuId, const uint32_t* buf, uint32_t len);
t_ftmPage* parseXml(const char* filename);
const char* msgIdFields[] = {"FID", "GID", "EVTNO", "SID", "BPID", "SCTR"};

xmlNode* checkNode(xmlNode* aNode, const char* name)
{
   int8_t i;
   i=0; while(name[i++] != '\0'); 
   
   if(aNode != NULL) 
   {
      while(aNode->type != XML_ELEMENT_NODE || (strncmp((const char*)aNode->name, name, i-1) != 0))
      { 
         aNode = xmlNextElementSibling(aNode);
         if(aNode == NULL) break;
      }
      return aNode;
   }
   return aNode;
}


t_ftmMsg* createMsg(xmlNode* msgNode, t_ftmMsg* pMsg)
{
   xmlNode *fieldNode, *subFieldNode = NULL;
   uint32_t i;
   
   fieldNode =  checkNode(msgNode->children, "id") ;
   if(fieldNode != NULL)
   {
         subFieldNode =  fieldNode->children;
         uint16_t vals[5];
         for(i=0;i<5;i++)
         {
            if( checkNode(subFieldNode, msgIdFields[i]) != NULL)
            {
               subFieldNode = checkNode(subFieldNode, msgIdFields[i]);
               vals[i] = (uint16_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL, 0 );
            }
            else printf("ERROR %s\n", msgIdFields[i]);
            subFieldNode =  xmlNextElementSibling(subFieldNode);
         }   
         pMsg->id = getId(vals[0], vals[1], vals[2], vals[3], vals[4], 0);
   } else printf("ERROR id %s \n",  fieldNode->name);
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "par");
   if(fieldNode != NULL) pMsg->par = (uint64_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   else printf("ERROR par\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "tef");
   if(fieldNode != NULL) pMsg->tef = (uint32_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   else printf("ERROR tef\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "offs");
   if(fieldNode != NULL) pMsg->offs = (uint64_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   else printf("ERROR offs\n");

   return pMsg;       
}

t_ftmCycle* createCyc(xmlNode* cycNode, t_ftmCycle* pCyc)
{
   
   xmlNode *fieldNode, *subFieldNode, *curNode = NULL;
   
   fieldNode =  checkNode(cycNode->children, "meta");
   if(fieldNode != NULL) fieldNode =  fieldNode->children;
   else printf("ERROR meta \n");
   
   
   fieldNode = checkNode(fieldNode, "rep");
   if(fieldNode != NULL) pCyc->repQty = (int32_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   else printf("ERROR repQty\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "period");
   if(fieldNode != NULL) pCyc->tPeriod = (uint64_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   else printf("ERROR Period\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "breakpoint");
   if(fieldNode != NULL) {if(strncmp( (const char*)xmlNodeGetContent(fieldNode), "yes",  3) == 0) pCyc->flags |= FLAGS_IS_BP;}
   else printf("ERROR breakpoint\n");
   
   curNode = fieldNode;
   fieldNode = checkNode(xmlNextElementSibling(curNode), "condition");
   if(fieldNode != NULL) 
   {
      subFieldNode = checkNode(fieldNode->children, "source");
      if(subFieldNode != NULL)
      {
               if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "shared",  6) == 0) pCyc->flags |= FLAGS_IS_COND_SHARED;
         else  if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "msi",     3) == 0) pCyc->flags |= FLAGS_IS_COND_MSI; 
      } else printf("ERROR source\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "pattern");
      if(subFieldNode != NULL) pCyc->condVal = (uint64_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL,  0 );
      else printf("ERROR condition\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "mask");
      if(subFieldNode != NULL) pCyc->condMsk = (uint64_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL,  0 );
      else printf("ERROR condmask\n");
   }
   else printf("no condition found\n");
   
   fieldNode = checkNode(xmlNextElementSibling(curNode), "signal");
   if(fieldNode != NULL)
   {
      pCyc->flags |= FLAGS_IS_SIG;
      subFieldNode = checkNode(fieldNode->children, "destination");
      if(subFieldNode != NULL) pCyc->sigDst = (uint32_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL,  0 );
      else printf("ERROR sigdst\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "value");
      if(subFieldNode != NULL) pCyc->sigVal = (uint32_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL,  0 );
      else printf("ERROR sigval\n");
   }
   else printf("no signal found \n");
         
   return pCyc;       
}


t_ftmPage* convertDOM2ftmPage(xmlNode * aNode)
{
   xmlNode *curNode, *pageNode, *planNode, *cycNode, *msgNode = NULL;
   
   t_ftmPage*  pPage    = NULL;
   t_ftmCycle* pCyc     = NULL;
   t_ftmCycle* pCycPrev = NULL;
   t_ftmCycle* pIdle    = NULL;
   t_ftmMsg* pMsg       = NULL;
   bool     planStart;
   uint32_t planIdx, cycIdx, msgIdx;
   uint32_t i;
   
   curNode = aNode;
   if(checkNode(curNode, "page") != NULL) pageNode = curNode;
   else return 0;

   planNode = pageNode->children;
   printf("PAGE\n");
   
   pPage    = calloc(1, sizeof(t_ftmPage));
   planIdx  = 0;
   
   while( checkNode(planNode, "plan") != NULL)
   {
      planNode = checkNode(planNode, "plan");
      printf("\tPLAN\n");
      cycIdx      = 0;
      cycNode     = planNode->children;
      planStart   = true;
      
      
      while( checkNode(cycNode, "cycle") != NULL)
      {
         //alloc cycle, fill first part from DOM
         cycNode  = checkNode(cycNode, "cycle");
         pCycPrev = pCyc;
         pCyc     = createCyc(cycNode, calloc(1, sizeof(t_ftmCycle)));
    
         printf("\t\tCYC\n");
         
         //if this is the first cycle of a plan, save the pointer for the plan array
         if(planStart) 
         {  
            planStart = false; 
            pPage->plans[planIdx].pStart = pCyc;
            
            curNode = checkNode(planNode->children, "starttime");
            if(curNode != NULL) 
            pCyc->tStart = (uint64_t)strtoul( (const char*)xmlNodeGetContent(curNode), NULL, 0 );
            
         } else {
            pCycPrev->pNext =  (struct t_ftmCycle*)pCyc;
         }
         
            
         //alloc msg array, fill array from DOM
         msgIdx = 0;
         pMsg = calloc(1024, sizeof(t_ftmMsg)); //alloc 1k msgs just in case
         msgNode = cycNode->children;
         while( checkNode(msgNode, "msg") != NULL)
         {
            msgNode = checkNode(msgNode, "msg");
            printf("\t\t\tMSG\n");
            createMsg(msgNode, &pMsg[msgIdx++]);
            msgNode = xmlNextElementSibling(msgNode);      
         }
         //realloc msg array size to actual space
         pMsg = realloc(pMsg, msgIdx*sizeof(t_ftmMsg)); //adjust msg mem allocation to actual cnt
         //write msg array ptr and msg qty to parent cycle
         pCyc->pMsg     = pMsg;
         pCyc->msgQty   = msgIdx;
         cycIdx++;
         cycNode = xmlNextElementSibling(cycNode);               
      }
      pCyc->pNext = (struct t_ftmCycle*)pIdle;
      pPage->plans[planIdx++].cycQty = cycIdx;
      
      planNode = xmlNextElementSibling(planNode);
   }
   pPage->planQty                 = planIdx;
   return pPage;    
}

void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, cycIdx, msgIdx;
   t_ftmCycle* pCyc  = NULL;
   t_ftmMsg*   pMsg  = NULL;
   
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      printf("\t---PLAN %c\n", planIdx+'A');
      cycIdx = 0;
      pCyc = pPage->plans[planIdx].pStart;
      while(cycIdx++ < pPage->plans[planIdx].cycQty && pCyc != NULL)
      {
         printf("\t\t---CYC %c%u\n", planIdx+'A', cycIdx-1);
         printf("\t\tStart:\t\t%08x%08x\n\t\tperiod:\t\t%08x%08x\n\t\trep:\t\t\t%08x\n\t\tmsg:\t\t\t%08x\n", 
         (uint32_t)(pCyc->tStart>>32), (uint32_t)pCyc->tStart, 
         (uint32_t)(pCyc->tPeriod>>32), (uint32_t)pCyc->tPeriod,
         pCyc->repQty,
         pCyc->msgQty);
         
         printf("\t\tFlags:\t");
         if(pCyc->flags & FLAGS_IS_BP) printf("-IS_BP\t");
         if(pCyc->flags & FLAGS_IS_COND_MSI) printf("-IS_CMSI\t");
         if(pCyc->flags & FLAGS_IS_COND_SHARED) printf("-IS_CSHA\t");
         if(pCyc->flags & FLAGS_IS_SIG) printf("-IS_SIG");
         printf("\n");
         
         printf("\t\tCondVal:\t%08x%08x\n\t\tCondMsk:\t%08x%08x\n\t\tSigDst:\t\t\t%08x\n\t\tSigVal:\t\t\t%08x\n", 
         (uint32_t)(pCyc->condVal>>32), (uint32_t)pCyc->condVal, 
         (uint32_t)(pCyc->condMsk>>32), (uint32_t)pCyc->condMsk,
         pCyc->sigDst,
         pCyc->sigVal);  
         
         pMsg = pCyc->pMsg;
         for(msgIdx = 0; msgIdx < pCyc->msgQty; msgIdx++)
         {
            printf("\t\t\t---MSG %c%u%c\n", planIdx+'A', cycIdx-1, msgIdx+'A');
            printf("\t\t\tid:\t%08x%08x\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id, 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par,
            pMsg[msgIdx].tef,
            (uint32_t)(pMsg[msgIdx].offs>>32), (uint32_t)pMsg[msgIdx].offs);   
         }
         pCyc = (t_ftmCycle*)pCyc->pNext;
      }
           
   }   
   
}


int eb_ram_com(const char* netaddress, uint8_t cpuId, const uint32_t* buf, uint32_t len)
{
   eb_socket_t socket;
   eb_status_t status;
   eb_device_t device;

   address_width  = EB_ADDRX;
   data_width     = EB_DATAX;
   int attempts   = 3;


   int num_devices;
   struct sdb_device devices[MAX_DEVICES];

   /* Default command-line arguments */

   address_width  = EB_ADDRX;
   data_width     = EB_DATAX;
   attempts       = 3;
   verbose        = 0;
   idx            = -1;
  
  /* open EB socket and device */
      if ((status = eb_socket_open(EB_ABI_CODE, 0, address_width|data_width, &socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }

  if ((status = eb_device_open(socket, netaddress, EB_ADDRX|EB_DATAX, attempts, &device)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }

  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no matching devices found\n", program);
    return 1;
  }

  if (num_devices > MAX_DEVICES) {
    fprintf(stderr, "%s: more devices found that tool supports (%d > %d)\n", program, num_devices, MAX_DEVICES);
    return 1;
  }

  if (idx > num_devices) {
    fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
    return 1;
  }

  if (idx == -1) {
    printf("Found %u devs\n", num_devices);
    for (idx = 0; idx < num_devices; ++idx) {
      printf("%.*s 0x%"PRIx64"\n", 19, &devices[idx].sdb_component.product.name[0], devices[idx].sdb_component.addr_first);
         if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0)
         printf("010\n");
    }
  } else {
    printf("0x%"PRIx64"\n", devices[idx].sdb_component.addr_first);
  }

  if ((status = eb_device_close(device)) != EB_OK) {
    fprintf(stderr, "%s: failed to close Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }

  if ((status = eb_socket_close(socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to close Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }
  
  return 0;
}

t_ftmPage* parseXml(const char* filename)
{
   xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
   t_ftmPage* pPage = NULL;

    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filename);
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    
    pPage = convertDOM2ftmPage(root_element);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();
    return pPage;

}

   int main(int argc, char** argv) {
      unsigned int i;
      int opt, error;
      t_ftmPage* pPage = NULL;
  
      const char* netaddress;
      char filename[64];
      
      error = 0;

      program = argv[0];
      /* Process the command-line arguments */
      while ((opt = getopt(argc, argv, "x:")) != -1) {
         switch (opt) {
            case 'x':
               i=0;
               while(i<64 && (optarg[i] != '\0')) { filename[i] = optarg[i]; i++;}
               filename[i] = '\0';
        
            case '?':
            fprintf(stderr, "Usage: %s  <protocol/host/port> -s <\"my string\">\n", program);
            break;
            default:
            fprintf(stderr, "%s: bad getopt result\n", program);
            return 1;
         }
      }

      if (optind + 1 != argc) {
         fprintf(stderr, "%s: Usage: simple-pRam<protocol/host/port> -x <\"myfile.xml\">\n", program);
         return 1;
      }

      program = argv[0];
      netaddress = argv[optind];

      printf("%s\n\n", filename);
      pPage = parseXml(filename);
      showFtmPage(pPage);
      //eb_ram_com(netaddress, 0, NULL, 0);
      printf("\n\n");

	
   return 0;
}


