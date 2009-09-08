#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pwd.h>

#include "r2m_config.h"
#include "getrss.h"

static void handler(int sig, siginfo_t *siginfo, void *unused)
{
  FILE *fp;
  time_t event;
  event = time(NULL);
  fp = fopen("/tmp/rss2mail.log", "a");
  if(!fp)
    perror("fopen()");
  fprintf(fp, "%sSIGHUP: reopen cfg file\n\n", ctime(&event));
  parse_config();
  fclose(fp);
  
}

int main(void)
{
  struct sigaction act;
  pid_t pid;
  int i = 1;
  struct MemoryStruct stuff;
  FILE *tmp;

  stuff.memory=NULL;
  stuff.size = 0;
  
  memset(&act, '\0', sizeof(act));

  act.sa_sigaction = &handler;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);

  if(sigaction(SIGHUP, &act, NULL))
    perror("sigaction():");
  pid = fork();
  if(pid == -1){
    perror("fork()\n");
    return -1;
    }
  else if(pid != 0){
    exit(EXIT_SUCCESS);
    }

  if(setsid() == -1)
    return -1;

  if(chdir("/") == -1)
    return -1;
  /*
  for(i = 0; i < NR_OPEN; i++)
    close(i);

  open("/dev/null", O_RDWR);
  dup(0);
  dup(0);
  */

  parse_config();
  
  //tmp = freopen("/tmp/output.txt", "a", stdout);

  while(1){
    printf("\n\n\n\t\t\tcounter = %d\t\t\t\n\n\n", i);
    getRss("http://www.opennet.ru/opennews/opennews_6.rss", (void*) &stuff);
    parseXml((void*) &stuff );
    sleep(1800);
    }

  if(stuff.memory)
    free(stuff.memory);

return 0;
}

