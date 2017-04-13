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
overflow(const char *src)
{
  char buf[BUFLEN];
  /* exploitable function */
  strcpy(&buf[0], src);
  /* nothing to do, just return */
}

int
main(int argc, char **argv)
{
  if (argc > 1) {
      overflow(argv[1]);
  } else {
    fprintf(stderr, "arg1 missing\n");
    return(1);
  }

  return (0);
}
