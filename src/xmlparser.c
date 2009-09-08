#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "getrss.h"

void parse_item_node(xmlDocPtr doc, xmlNodePtr item)
{
        xmlNode *cur;
        xmlChar *text;

        printf("===================\n");

        for (cur = item->xmlChildrenNode; cur; cur = cur->next) {
                if (cur->type == XML_ELEMENT_NODE) {
                        text = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                        printf("%s:\n%s\n", cur->name, text);

                }
        }

        printf("========================\n");
}

int parseXml(void* stuffptr)
{
	struct MemoryStruct *xmlptr = (struct MemoryStruct *)stuffptr;

        xmlDocPtr doc;
        xmlNode *cur, *root;


        doc = xmlParseMemory(xmlptr->memory, xmlptr->size);
        if(!doc){
          printf("Document not parsed successfully.\n");
          return 1;
        }

        root = xmlDocGetRootElement(doc);
        if (!root) {
          printf("empty document\n");
          xmlFreeDoc(doc);
          return 1;
        }

        /* walk through top-level elements searching for items */
        for (cur = root; cur; cur = cur->next) {
          if (cur->type == XML_ELEMENT_NODE) {
            if (strcmp("rss", cur->name) == 0) {
              xmlNode *cur2;

              for (cur2 = cur->xmlChildrenNode; cur2; cur2 = cur2->next) {
                if (cur2->type == XML_ELEMENT_NODE) {
                  if (strcmp(cur2->name, "channel") == 0) {
                    xmlNode *cur3;

                    for (cur3 = cur2->xmlChildrenNode; cur3; cur3 = cur3->next) {
                      if (cur3->type == XML_ELEMENT_NODE)
                        if (strcmp("item", cur3->name) == 0)
                          parse_item_node(doc, cur3);
                    }
                  }
                }
              }
            }
          }
        }

        xmlFreeDoc(doc);

        return 0;
}

