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
#define BUFLEN    256

u_char buf[BUFLEN];
u_char *env;

int
main()
{
  if (env = getenv(ENV_VAR))
    {
      fprintf(stderr, "env: "ENV_VAR" set\n");
      fprintf(stderr, "env: %s\n", env);

      strcpy(buf, env);
    }
  else
    {
      fprintf(stderr, "env: "ENV_VAR" not set, abort!\n");
    }

  return 0;
}
