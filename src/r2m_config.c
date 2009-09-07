#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <json.h>

#define CONFIG_PATH	"/.rss2mailrc"

#include "r2m_config.h"

char*  get_cfg_path(void)
{
        static char* strptr;
        size_t cnt;
	struct passwd* user;

	user = getpwuid(getuid());
	
        cnt = strlen(user->pw_dir) + sizeof(CONFIG_PATH);
        strptr = (char*)malloc(cnt);
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

	/* TODO: lazy initialization, huh? probably not a good thing to do in C... */
	if (r2m_config == NULL)
		r2m_config = (struct r2m_config_t*)malloc(sizeof(struct r2m_config_t*));

	if (r2m_config->smtp == NULL)
		/* allocate smtp stuff as well */
		r2m_config->smtp = (struct r2m_smtp_t*)malloc(sizeof(struct r2m_smtp_t*));

	root = json_tokener_parse(content);
	if (is_error(root)) {
		printf("error parsing config file!\n");
		exit(1);
	}

	obj = json_object_object_get(root, "email");

	r2m_config->email = json_object_get_string(obj);

	obj = json_object_object_get(root, "feeds");

	for (i = 0; i < json_object_array_length(obj); i++) {
		struct json_object *item = json_object_array_get_idx(obj, i);
		printf("%s\n", json_object_get_string(item));
	}

	obj = json_object_object_get(root, "smtp");

	r2m_config->smtp->host = json_object_get_string(json_object_object_get(obj, "host"));
	r2m_config->smtp->login = json_object_get_string(json_object_object_get(obj, "login"));
	r2m_config->smtp->password = json_object_get_string(json_object_object_get(obj, "password"));
}
