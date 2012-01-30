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
#define BUFLEN    64

char buf[BUFLEN];
char *env;

int
main()
{
  fprintf(stderr, "buflen: %d\nenv_var: %s\n\n", BUFLEN, ENV_VAR);
  if ( (env = getenv(ENV_VAR)) )
    {
      fprintf(stderr, "env_var: "ENV_VAR"\n");
      fprintf(stderr, "env: %s\n", env);

      strcpy(buf, env);
    }
  else
    {
      fprintf(stderr, "env_var: "ENV_VAR" not set, abort!\n");
    }

  return(0);
}
