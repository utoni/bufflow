#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  char buf[30];
  /* exploitable function */
  strcpy(buf, argv[1]);
  return 0;
}
