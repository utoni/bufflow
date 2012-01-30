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
#define BUFLEN    2

char buf[BUFLEN];
char *env;

int
main(int argc, char **argv)
{
  fprintf(stderr, "buflen: %d\nenv_var: %s\nargs: %d\n\n", BUFLEN, ENV_VAR, (argc - 1));
  if (argc > 1)
    {
      fprintf(stderr, "arg0: %s\n", argv[1]);

      strcpy(buf, argv[1]);
    }
  else if ((env = getenv(ENV_VAR)))
    {
      fprintf(stderr, "env_var: "ENV_VAR"\n");
      fprintf(stderr, "env: %s\n", env);

      strcpy(buf, env);
    }
  else
    {
      fprintf(stderr, "neither env_var ("ENV_VAR") set or arg0 given, abort!\n");
      return(1);
    }

  printf("buf: %p\n", buf);

  return (0);
}
