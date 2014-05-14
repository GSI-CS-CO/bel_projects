#ifndef _XMLAUX_H_
#define _XMLAUX_H_
#include "ftmx86.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


typedef enum { false = 0, true = 1 } bool;
extern const char* msgIdFields[];

uint64_t strtou64(const char* s);
xmlNode*    checkNode(xmlNode* aNode, const char* name);
t_ftmMsg*   createMsg(xmlNode* msgNode, t_ftmMsg* pMsg);
t_ftmChain* createChain(xmlNode* chainNode, t_ftmChain* pChain);
t_ftmPage*  createPage(xmlNode* pageNode, t_ftmPage* pPage);
t_ftmPage*  convertDOM2ftmPage(xmlNode * aNode);
t_ftmPage* parseXml(const char* filename);

#endif
