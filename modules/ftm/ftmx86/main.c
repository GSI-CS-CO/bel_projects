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
   
   while(aNode != NULL) 
   {
      while(aNode->type != XML_ELEMENT_NODE || (strncmp((const char*)aNode->name, name, i-1) != 0))
      { 
         aNode = xmlNextElementSibling(aNode);
      }
      return aNode;
   }
   return aNode;
}


t_ftmMsg* createMsg(xmlNode* msgNode, t_ftmMsg* pMsg)
{
   xmlNode *fieldNode, *subFieldNode = NULL;
   uint32_t i;
   
   
   fieldNode =  msgNode->children;
   if( checkNode(fieldNode, "id") )
   {
         fieldNode = checkNode(fieldNode, "id");
         subFieldNode =  fieldNode->children;
         uint16_t vals[6];
         for(i=0;i<6;i++)
         {
            if( checkNode(subFieldNode, msgIdFields[i]) != NULL)
            {
               subFieldNode = checkNode(subFieldNode, msgIdFields[i]);
               vals[i] = (uint16_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL, 0 );
            }
            else printf("ERROR %s\n", msgIdFields[i]);
            subFieldNode =  xmlNextElementSibling(subFieldNode);
         }   
         pMsg->id = getId(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
   } else printf("ERROR id %s \n",  fieldNode->name);
   fieldNode =  xmlNextElementSibling(fieldNode);

   if( checkNode(fieldNode, "par") != NULL) 
   {
      fieldNode = checkNode(fieldNode, "par");
      pMsg->par = (uint64_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   }  
   else printf("ERROR par\n");
   fieldNode =  xmlNextElementSibling(fieldNode);

   if( checkNode(fieldNode, "tef") != NULL) 
   {
      fieldNode = checkNode(fieldNode, "tef");
      pMsg->tef = (uint32_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   }     
   else printf("ERROR tef\n");
   fieldNode =  xmlNextElementSibling(fieldNode);

   if( checkNode(fieldNode, "offs") != NULL) 
   {
      fieldNode = checkNode(fieldNode, "offs");
      pMsg->offs = (uint64_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  0 );
   }
   else printf("ERROR offs\n");

/*
   printf("id:\t%04x%04x\npar:\t%04x%04x\ntef:\t%04x\noffs:\t%04x%04x\n", 
         (uint32_t)(pMsg->id>>32), (uint32_t)pMsg->id, 
         (uint32_t)(pMsg->par>>32), (uint32_t)pMsg->par,
         pMsg->tef,
         (uint32_t)(pMsg->offs>>32), (uint32_t)pMsg->offs);
  */       
   return pMsg;       
}

t_ftmCycle* createCyc(xmlNode* cycNode, t_ftmCycle* pCyc)
{
   /*
   xmlNode *fieldNode, *subFieldNode = NULL;
   uint32_t i;
   
   
   fieldNode =  msgNode->children;
   if( checkNode(fieldNode, "id") )
   {
         fieldNode = checkNode(fieldNode, "id");
         subFieldNode =  fieldNode->children;
         uint16_t vals[6];
         for(i=0;i<6;i++)
         {
            if( checkNode(subFieldNode, msgIdFields[i]) != NULL)
            {
               subFieldNode = checkNode(subFieldNode, msgIdFields[i]);
               vals[i] = (uint16_t)strtoul( (const char*)xmlNodeGetContent(subFieldNode), NULL, 10 );
            }
            else printf("ERROR %s\n", msgIdFields[i]);
            subFieldNode =  xmlNextElementSibling(subFieldNode);
         }   
         pMsg->id = getId(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
   } else printf("ERROR id %s \n",  fieldNode->name);
   fieldNode =  xmlNextElementSibling(fieldNode);

   

   if( checkNode(fieldNode, "offs") != NULL) 
   {
      fieldNode = checkNode(fieldNode, "offs");
      pMsg->offs = (uint64_t)strtoul( (const char*)xmlNodeGetContent(fieldNode), NULL,  10 );
   }
   else printf("ERROR offs\n");


   */
         
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

      while( checkNode(cycNode, "cyc") != NULL)
      {
         //alloc cycle, fill first part from DOM
         cycNode  = checkNode(cycNode, "cyc");
         pCycPrev = pCyc;
         pCyc     = calloc(1, sizeof(t_ftmCycle));
         //createCyc(cycNode, &pCyc);
         printf("\t\tCYC\n");
         
         //if this is the first cycle of a plan, save the pointer for the plan array
         if(planStart) 
         {  
            planStart = false; 
            pPage->plans[planIdx].pStart = pCyc;
            
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
         printf("\t\t---CYC %c%u\n", planIdx+'A', cycIdx);
         printf("\t\tStart:\t%04x%04x\n\t\tperiod:\t%04x%04x\n\t\tflags:\t%04x\n\t\trep:\t%04x\n\t\tmsg:\t%04x\n", 
         (uint32_t)(pCyc->tStart>>32), (uint32_t)pCyc->tStart, 
         (uint32_t)(pCyc->tPeriod>>32), (uint32_t)pCyc->tPeriod,
         pCyc->flags,
         pCyc->repQty,
         pCyc->msgQty); 
         
         pMsg = pCyc->pMsg;
         for(msgIdx = 0; msgIdx < pCyc->msgQty; msgIdx++)
         {
            printf("\t\t\t---MSG %c%u%c\n", planIdx+'A', cycIdx, msgIdx+'A');
            printf("\t\t\tid:\t%04x%04x\n\t\t\tpar:\t%04x%04x\n\t\t\ttef:\t%04x\n\t\t\toffs:\t%04x%04x\n", 
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


