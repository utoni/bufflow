/*
 * overflow.c
 *
 *  Created on: 27.01.2012
 *      Author: druid
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ENV_VAR   "EXPLOIT"
#define BUFLEN    10


void
overflow(const char *src, char *dst)
{
  /* exploitable function */
  strcpy(dst, src);
}

int
main(int argc, char **argv)
{
  char *s, *env;
  char buf[BUFLEN];

  fprintf(stderr, "buflen: %d\nenv_var: %s\nargs: %d\n\n", BUFLEN, ENV_VAR, (argc - 1));
  if (argc > 1)
    {
      overflow(argv[1], buf);
    }
  else if ((env = getenv(ENV_VAR)))
    {
      overflow(env, buf);
    }
  else
    {
      fprintf(stderr, "neither "ENV_VAR" set or arg0 given, abort!\n");
      return(1);
    }

  printf("buf: %s\n*buf: %p\nbuflen: %d\n", s, s, strlen(s));

  return (0);
}
