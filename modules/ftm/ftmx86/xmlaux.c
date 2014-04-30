#include "xmlaux.h"

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

t_ftmPage* createPage(xmlNode* pageNode, t_ftmPage* pPage)
{
   
   xmlNode *fieldNode = NULL;
   const char* planChar;
   
   fieldNode =  checkNode(pageNode->children, "meta");
   if(fieldNode != NULL) fieldNode =  fieldNode->children;
   else printf("ERROR meta \n");
   
   
   fieldNode = checkNode(fieldNode, "startplan");
   if(fieldNode != NULL) 
   {  
      planChar = (const char*)xmlNodeGetContent(fieldNode);
      if (strcmp(planChar, "idle") != 0) pPage->idxStart = 0xdeadbeef;
      else pPage->idxStart = (uint32_t)(planChar[0] & 0xdf) - 'A';
   }
   else printf("ERROR startplan\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "altplan");
   if(fieldNode != NULL)
   {  
      planChar = (const char*)xmlNodeGetContent(fieldNode);
      if (strcmp(planChar, "idle") != 0) pPage->idxBp = 0xdeadbeef;
      else pPage->idxBp = (uint32_t)(planChar[0] & 0xdf) - 'A';
   }
   else printf("ERROR altplan\n");
   return pPage;       
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
   
   curNode = aNode;
   
   if(checkNode(curNode, "page") != NULL) pageNode = curNode;
   else return 0;
   planNode = pageNode->children;
   printf("PAGE\n");
   
   pPage    = createPage(pageNode, calloc(1, sizeof(t_ftmPage)));
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
      pCyc->flags |= FLAGS_IS_END;
      planNode = xmlNextElementSibling(planNode);
   }
   pPage->planQty                 = planIdx;
   
   return pPage;    
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
