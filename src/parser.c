#include <stdio.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

void parsexml(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while(cur != NULL){
	  key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	  if(key != NULL)
	    printf("%s\n", key);
	  xmlFree(key);
	  cur = cur->next;
	}
}

int main(int argc,char** argv)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	if(argc != 2){
	  printf("Usage: parser <xml-file>\n");
	  return 1;
	}
	
	doc = xmlParseFile(argv[1]);
	if(!doc){
	  printf("Document not parsed successfully.\n");
	  return 1;
	}

	cur = xmlDocGetRootElement(doc);
	if(!cur){
	  printf("empty document\n");
	  xmlFreeDoc(doc);
	  return 1;
	}
	
	cur = cur->xmlChildrenNode;
	while(cur != NULL){
	  parsexml(doc, cur);
	  cur = cur->next;
	}

	xmlFreeDoc(doc);

return 0;
}
