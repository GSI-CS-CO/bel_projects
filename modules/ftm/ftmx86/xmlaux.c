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

uint64_t strtou64(const char* s)
{
   int base;
   uint64_t ret;
   
   if(strstr(s, "0x") != NULL) { base = 16; }
   else base = 10;
   ret = strtoull( s, NULL, base );
   
    //printf("TEST b %i t %s n %llu\n", base, s, ret);
   
   return ret;
}


t_ftmMsg* createMsg(xmlNode* msgNode, t_ftmMsg* pMsg)
{
   xmlNode *fieldNode, *subFieldNode = NULL;
   uint32_t i;
   uint64_t offset;
   uint16_t vals[5];
   
   fieldNode =  checkNode(msgNode->children, "id") ;
   if(fieldNode != NULL)
   {
         subFieldNode =  fieldNode->children;
         
         //printf("MSG NODE\n Vals:\t");
         for(i=0;i<5;i++)
         {
            if( checkNode(subFieldNode, msgIdFields[i]) != NULL)
            {
               subFieldNode = checkNode(subFieldNode, msgIdFields[i]);
               vals[i] = (uint16_t)strtou64( (const char*)xmlNodeGetContent(subFieldNode));
            }
            else printf("ERROR %s\n", msgIdFields[i]);
            subFieldNode =  xmlNextElementSibling(subFieldNode);
            //printf("\t %02x", vals[i]);
         }   
         pMsg->id = getId(vals[0], vals[1], vals[2], vals[3], vals[4], 0);
   } else printf("ERROR id %s \n",  fieldNode->name);
   //printf("\n");
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "par");
   if(fieldNode != NULL) pMsg->par = strtou64( (const char*)xmlNodeGetContent(fieldNode));
   else printf("ERROR par\n");
   
   //FIXME eca v1 only

   //fieldNode = checkNode(xmlNextElementSibling(fieldNode), "tef");
   //if(fieldNode != NULL) pMsg->tef = strtou64( (const char*)xmlNodeGetContent(fieldNode));
   //else printf("ERROR tef\n");   
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "offs");
   
   if(fieldNode != NULL) offset = strtou64( (const char*)xmlNodeGetContent(fieldNode));
   else {offset = 0; printf("ERROR offs\n");}
   
   pMsg->offs  = offset;// >>3;           //offset is a multiple of 8ns
   //OBSOLOTE WITH ECA2 - DM USES NANOSECONDS NATIVELY NOW!!!	
   //pMsg->tef   = ((uint32_t)offset & 0x7) << 29;  //Tef is 0-7ns, but left shifted because it's a fixed point fraction
   
   return pMsg;       
}

t_ftmChain* createChain(xmlNode* chainNode, t_ftmChain* pChain)
{
   
   xmlNode *fieldNode, *subFieldNode, *curNode = NULL;
   
   fieldNode =  checkNode(chainNode->children, "meta");
   if(fieldNode != NULL) fieldNode =  fieldNode->children;
   else printf("ERROR meta \n");
   
   
   fieldNode = checkNode(fieldNode, "rep");
   if(fieldNode != NULL) pChain->repQty = (int32_t)strtou64( (const char*)xmlNodeGetContent(fieldNode));
   else printf("ERROR repQty\n");
   /*
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "persistent");
   if(fieldNode != NULL) {if(strncmp( (const char*)xmlNodeGetContent(fieldNode), "yes",  3) == 0) pChain->flags |= FLAGS_IS_PERS_REP_CNT;}
   else printf("ERROR persistent cnt\n");
   */
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "period");
   if(fieldNode != NULL) pChain->tPeriod = (uint64_t)strtou64( (const char*)xmlNodeGetContent(fieldNode));
   else printf("ERROR Period\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "branchpoint");
   if(fieldNode != NULL) {if(strncmp( (const char*)xmlNodeGetContent(fieldNode), "yes",  3) == 0) pChain->flags |= FLAGS_IS_BP;}
   else printf("ERROR branchpoint\n");
   
   curNode = fieldNode;
   fieldNode = checkNode(xmlNextElementSibling(curNode), "condition");
   if(fieldNode != NULL) 
   {
      subFieldNode = checkNode(fieldNode->children, "source");
      if(subFieldNode != NULL)
      {
               if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "shared",  6) == 0) pChain->flags |= FLAGS_IS_COND_SHARED;
         else  if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "msi",     3) == 0) pChain->flags |= FLAGS_IS_COND_MSI;
         else  if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "0x",      2) == 0) {strtou64( (const char*)xmlNodeGetContent(subFieldNode));
         pChain->flags |= FLAGS_IS_COND_ADR;} 
      } else printf("ERROR cond source\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "pattern");
      if(subFieldNode != NULL) pChain->condVal = strtou64( (const char*)xmlNodeGetContent(subFieldNode));
      else printf("ERROR cond pattern\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "mask");
      if(subFieldNode != NULL) pChain->condMsk = strtou64( (const char*)xmlNodeGetContent(subFieldNode));
      else printf("ERROR condmask\n");
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "always");
      if(subFieldNode != NULL) if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "yes",  3) == 0) pChain->flags |= FLAGS_IS_COND_ALL;
      
   }
   //else printf("no condition found\n");
   
   fieldNode = checkNode(xmlNextElementSibling(curNode), "signal");
   if(fieldNode != NULL)
   {
      subFieldNode = checkNode(fieldNode->children, "destination");
      if(subFieldNode != NULL)
      {
               if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "shared",  6) == 0) pChain->flags |= FLAGS_IS_SIG_SHARED;
         else  if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "msi",     3) == 0) pChain->flags |= FLAGS_IS_SIG_MSI;
         else  if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "0x",      2) == 0) {strtou64( (const char*)xmlNodeGetContent(subFieldNode));
         pChain->flags |= FLAGS_IS_SIG_ADR;}
      } else printf("ERROR sig destination\n");
          
      
      subFieldNode = checkNode(fieldNode->children, "cpu");
      if(subFieldNode != NULL) pChain->sigCpu = (uint32_t)strtou64( (const char*)xmlNodeGetContent(subFieldNode));
      else printf("ERROR sig cpu\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "value");
      if(subFieldNode != NULL) pChain->sigVal = (uint32_t)strtou64( (const char*)xmlNodeGetContent(subFieldNode));
      else printf("ERROR sig val\n");
      
      subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "always");
      if(subFieldNode != NULL) if(strncmp( (const char*)xmlNodeGetContent(subFieldNode), "yes",  3) == 0) pChain->flags |= FLAGS_IS_SIG_ALL;
       
      
   }
   //else printf("no signal found \n");
         
   return pChain;       
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
      if (strcmp(planChar, "idle") == 0) pPage->idxStart = 0xdeadbeef;
      else pPage->idxStart = (uint32_t)(planChar[0] & 0xdf) - 'A';
   }
   else printf("ERROR startplan\n");
   
   fieldNode = checkNode(xmlNextElementSibling(fieldNode), "altplan");
   if(fieldNode != NULL)
   {  
      planChar = (const char*)xmlNodeGetContent(fieldNode);
      if (strcmp(planChar, "idle") == 0) pPage->idxBp = 0xdeadbeef;
      else pPage->idxBp = (uint32_t)(planChar[0] & 0xdf) - 'A';
   }
   else printf("ERROR altplan\n");
   return pPage;       
}


t_ftmPage* convertDOM2ftmPage(xmlNode * aNode)
{
   xmlNode *curNode, *pageNode, *planNode, *fieldNode, *subFieldNode, *chainNode, *msgNode = NULL;
   
   t_ftmPage*  pPage    = NULL;
   t_ftmChain* pChain     = NULL;
   t_ftmChain* pChainPrev = NULL;
   t_ftmChain* pIdle    = NULL;
   t_ftmMsg* pMsg       = NULL;
   bool     planStart, planIsLoop = 0;
   uint32_t planIdx, chainIdx, msgIdx;
   uint64_t starttime = 0;
   
   curNode = aNode;
   
   if(checkNode(curNode, "page") != NULL) pageNode = curNode;
   else return 0;
   planNode = pageNode->children;
  // printf("PAGE\n");
   
   pPage    = createPage(pageNode, calloc(1, sizeof(t_ftmPage)));
   planIdx  = 0;
 
   while( checkNode(planNode, "plan") != NULL)
   {
      planNode = checkNode(planNode, "plan");
      //printf("\tPLAN\n");
      
      chainIdx      = 0;
      
      planStart   = true;
   
   const char* planChar;
   
   fieldNode =  checkNode(planNode->children, "meta");
   subFieldNode = NULL;
   if(fieldNode != NULL) subFieldNode = fieldNode->children;
   else printf("ERROR meta \n");
   
   
   subFieldNode = checkNode(subFieldNode, "starttime");
   if(subFieldNode != NULL) 
   {  
       starttime = strtou64( (const char*)xmlNodeGetContent(subFieldNode));
   }
   else printf("ERROR starttime\n");
   
   subFieldNode = checkNode(xmlNextElementSibling(subFieldNode), "lastjump");
   if(subFieldNode != NULL)
   {  
      planChar = (const char*)xmlNodeGetContent(subFieldNode);
      if (strcmp(planChar, "self") == 0) { planIsLoop = 1; }
      else planIsLoop = 0;
   }
   else printf("ERROR lastjump\n");
   
   chainNode = xmlNextElementSibling(fieldNode);   

      while( checkNode(chainNode, "chain") != NULL)
      {
         //alloc chain, fill first part from DOM
         chainNode  = checkNode(chainNode, "chain");
         pChainPrev = pChain;
         pChain     = createChain(chainNode, calloc(1, sizeof(t_ftmChain)));
    
         //printf("\t\tCYC\n");
         
         //if this is the first chain of a plan, save the pointer for the plan array
         if(planStart) 
         {  
            planStart = false; 
            pPage->plans[planIdx].pStart = pChain;
            pChain->tStart = starttime;
         } else {
            pChainPrev->pNext =  (struct t_ftmChain*)pChain;
         }
         
         //alloc msg array, fill array from DOM
         msgIdx = 0;
         pMsg = calloc(1024, sizeof(t_ftmMsg)); //alloc 1k msgs just in case
         msgNode = chainNode->children;
         while( checkNode(msgNode, "msg") != NULL)
         {
            msgNode = checkNode(msgNode, "msg");
            //printf("\t\t\tMSG\n");
            createMsg(msgNode, &pMsg[msgIdx++]);
            msgNode = xmlNextElementSibling(msgNode);      
         }
         //realloc msg array size to actual space
         pMsg = realloc(pMsg, msgIdx*sizeof(t_ftmMsg)); //adjust msg mem allocation to actual cnt
         //write msg array ptr and msg qty to parent chain
         pChain->pMsg     = pMsg;
         pChain->msgQty   = msgIdx;
         chainIdx++;
         chainNode = xmlNextElementSibling(chainNode);               
      }
      if(planIsLoop) { printf("Plan %u loops to Ptr 0x%p\n", planIdx, pPage->plans[planIdx].pStart);
                       pChain->flags |= FLAGS_IS_ENDLOOP;
                       pChain->pNext = (struct t_ftmChain*)(pPage->plans[planIdx].pStart);
                     }  
      else {pChain->pNext = (struct t_ftmChain*)pIdle;}
      pPage->plans[planIdx++].chainQty = chainIdx;
      pChain->flags |= FLAGS_IS_END;
      
      planNode = xmlNextElementSibling(planNode);
   }
   pPage->planQty                 = planIdx;
   
   return pPage;    
}

t_ftmPage* parseXmlFile(const char* filename)
{
   xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
   t_ftmPage* pPage = NULL;

    //qLIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filename);
        return NULL;
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

t_ftmPage* parseXmlString(const char* sXml)
{
   xmlDoc *doc = NULL;
   xmlNode *root_element = NULL;
   t_ftmPage* pPage = NULL;

    //LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlParseDoc((const xmlChar *)sXml);

    if (doc == NULL) {
        fprintf(stderr, "error: could not parse string \n");
        return NULL;
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

