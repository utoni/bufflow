//	#DECODER=./simple_decoder.o
//	#SHELLCODE=../hello.o
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _USE_CFG
#include "simple_encoder.h"
#else
#error "simple_encode.h config file missing including decoder && shellcode"
#endif

#ifndef _CRYPTVAL
#define _CRYPTVAL 200
#endif


int
getnumber(int n)
{
  int seed;
  struct timeval tm;

  gettimeofday(&tm, NULL);
  seed = tm.tv_sec + tm.tv_usec;
  srandom(seed);
  return (random() % n);
}

void
print_code(char *data)
{
  int i,l = 15;

  printf("\n\nunsigned long int lshellcode = %lu;\nchar shellcode[] = \n", (unsigned long int) strlen(data));
  for (i = 0; i < strlen(data); i++) {
    if (l >= 15) {
      if (i) {
        printf("\"\n");
      }
      printf("\t\"");
      l = 0;
    }
    ++l;
    printf("\\x%02x", ((unsigned char *)data)[i]);
  }
  printf("\";\n\n\n");
}

int
main(int argc, char **argv)
{
//  char decoder[] = _DECODER;
  int count, number = getnumber(_CRYPTVAL), nullbyte = 0, ldecoder = strlen(decoder), lshellcode = strlen(shellcode);
  char *result;

  printf("Using value %d to encode the shellcode.\n", number);
  printf("*** PRINT SHELLCODE\n");
  print_code(shellcode);
  printf("*** PRINT DECODER\n");
  print_code(decoder);
}
