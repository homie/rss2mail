#include <stdio.h>

#include "r2m_config.h"

char** envvar;

int main(int argc, char** argv, char** envp)
{
	envvar = envp;

	printf("hello world\n");
	
	parse_config();

	return 0;
}
