#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <json.h>

#define CONFIG_PATH	"/.rss2mailrc"

char*  get_cfg_path(void)
{
        static char* strptr;
        size_t cnt;
	struct passwd* user;

	user = getpwuid(getuid());
	
        cnt = strlen(user->pw_dir) + sizeof(CONFIG_PATH);
	if(!strptr)
          strptr = (char*)malloc(cnt);
	else
	  strptr = (char*)realloc((void*)strptr, cnt);
	snprintf(strptr, cnt, "%s%s", user->pw_dir, CONFIG_PATH);
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
