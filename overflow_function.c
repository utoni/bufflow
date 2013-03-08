#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stupid(char *str)
{
  char buf[30];
  /* exploitable function */
  strcpy(buf, str);
}

int main(int argc, char **argv)
{
  stupid(argv[1]);
  return 0;
}
