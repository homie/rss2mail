#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};


static void *myrealloc(void *ptr, size_t size)
{
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

int getRss(char* link, void *stuffptr)
{
	CURL *curl_handle;
	struct MemoryStruct *chunk = (struct MemoryStruct *)stuffptr;

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, link);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, chunk);

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  
	curl_easy_perform(curl_handle);

	chunk->memory[chunk->size] = 0;

  
	curl_easy_cleanup(curl_handle);


	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	return 0;
}

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

int main(int argc, char** argv)
{
	struct MemoryStruct stuff;
	stuff.memory=NULL; /* we expect realloc(NULL, size) to work */
	stuff.size = 0;    /* no data at this point */
	
	xmlDocPtr doc;
        xmlNode *cur, *root;
	
	if(argc != 2){
	  printf("Usage: %s <rss-channel>\n", argv[0]);
	  return 1;
	}

	getRss(argv[1], (void*) &stuff);

	//printf("Data:%s\n", stuff.memory);
	
	parseXml((void*) &stuff );

	if(stuff.memory)
	  free(stuff.memory);
return 0;
}
