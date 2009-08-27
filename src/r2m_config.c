#include <stdio.h>
#include <stdlib.h>
#include <json.h>

#define CONFIG_PATH	"/.rss2mailrc"

char*  get_cfg_path(void)
{
        int i;
        char* strptr, *envpath;
        size_t cnt;

	envpath = getenv("HOME");

	
        cnt = strlen(envpath) + sizeof(CONFIG_PATH);
        strptr = (char*)malloc(cnt);
	snprintf(strptr, cnt, "%s%s", envpath, CONFIG_PATH);
        return strptr;
}


int load_file_to_mem(const char *filename, char **result)
{
	long size = 0;

	FILE *f = fopen(filename, "rb");

	if (f == NULL) {
		*result = NULL;
		return -1;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char*)malloc(size + 1);
	if (size != fread(*result, sizeof(char), size, f)) {
		free(*result);
		return -1;
	}

	fclose(f);
	(*result)[size] = 0;
	return size;
}

void parse_config()
{
	char *content;
	int size, i;
	struct json_object *obj, *root;
	
	size = load_file_to_mem(get_cfg_path(), &content);

//	printf("%d\n", size);
//	printf("==\n%s\n==\n", content);

	root = json_tokener_parse(content);
	obj = json_object_object_get(root, "email");

	printf("email: %s\n", json_object_get_string(obj));

	obj = json_object_object_get(root, "feeds");

	printf("feeds:\n");
	for (i = 0; i < json_object_array_length(obj); i++) {
		struct json_object *item = json_object_array_get_idx(obj, i);
		printf("%s\n", json_object_get_string(item));
	}

}
