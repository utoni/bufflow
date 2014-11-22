//	#DECODER=./simple_decoder.o
//	#SHELLCODE=../shellcode/hello.o
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
print_code(const char *name, char *data, int len)
{
  int i,l = 15;

  printf("unsigned long int l%s = %lu;\nchar %s[] = \n", name, (unsigned long int) strlen(data), name);
  for (i = 0; i < len; i++) {
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
  printf("\";\n\n");
}

int
main(int argc, char **argv)
{
  int i, npos = 0, number = getnumber(_CRYPTVAL), nullbyte = 0;
  int ldecoder = sizeof(decoder)-1; /* last byte is '\x00' */
  int lshellcode = sizeof(shellcode)-1; /* same as above */
  int first_arg = 1;
  char *result;

  printf("/* Using value %d to encode the shellcode. */\n", number);
  printf("/* PRINT SHELLCODE */\n");
  print_code("shellcode", shellcode, lshellcode);
  printf("/* PRINT DECODER */\n");
  print_code("decoder", decoder, ldecoder);

  for (i = 0; i < ldecoder; i++) {
    if (decoder[i] == '\x00') {
      if (first_arg) {
        decoder[i] = lshellcode;
        first_arg = 0;
      } else {
        decoder[i] = (unsigned char) number;
        npos = i;
      }
      printf("// decoder[%d] = %u (%02x)\n", i, (unsigned char) decoder[i], (unsigned char) decoder[i]);
    }
  }

  do {
    if (nullbyte == 1) {
      number = getnumber(10);
      decoder[npos] += number;
      nullbyte = 0;
    }

    for (i = 0; i < lshellcode; i++) {
      shellcode[i] += number;
      if (shellcode[i] == '\x00') {
        nullbyte = 1;
      }
    }
  } while (nullbyte == 1);

  return (0);
}
