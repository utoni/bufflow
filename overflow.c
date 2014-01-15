/*
 * overflow.c
 *
 *  Created on: 27.01.2012
 *      Author: druid
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 300 bytes buffer len + 4 bytes for overwrite return opcode */
#define BUFLEN    300

void
overflow(const char *src, char *dst)
{
  /* exploitable function */
  strcpy(dst, src);
  /* nothing to do, just return */
}

int
main(int argc, char **argv)
{
  char buf[BUFLEN];

  if (argc > 1) {
      overflow(argv[1], buf);
  } else {
    fprintf(stderr, "arg1 missing\n");
    return(1);
  }

  return (0);
}
