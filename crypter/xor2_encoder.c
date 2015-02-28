//	#DECODER=./xor_decoder.o
//	#SHELLCODE=../shellcode/hello.o
#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _USE_CFG
#include "xor2_encoder.h"
#else
#error "xor2_encoder.h config file missing including decoder && shellcode"
#endif

#ifndef _CRYPTVAL
#define _CRYPTVAL 0xff
#endif

#ifndef _OUTFILE
#define _OUTFILE "xor2_encoded.o"
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

void
err_n_xit(const char *exit_msg, const char *arg)
{
  char *tmp;
  if (arg != NULL) {
    asprintf(&tmp, "%s('%s')", exit_msg, arg);
  } else {
    tmp = (char *) exit_msg;
  }
  perror(tmp);
  if (arg != NULL) {
    free(tmp);
  }
  exit(1);
}

int
main(int argc, char **argv)
{
  int i, npos = 0, number = getnumber(_CRYPTVAL), nullbyte = 0;
  int ldecoder = sizeof(decoder)-1; /* last byte is '\x00' */
  int lshellcode = sizeof(shellcode)-1; /* same as above */
  int first_arg = 1;
  char *result;
  FILE *outfile;

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
  printf("\n");

  result = malloc(lshellcode);
  do {
    memcpy(result, shellcode, lshellcode);

    if (nullbyte == 1) {
      number = getnumber(_CRYPTVAL);
      fprintf(stderr, "New crypt value: %d (%02x)\n", number, number);
      decoder[npos] = number;
      nullbyte = 0;
    }

    for (i = 0; i < lshellcode; i++) {
      result[i] ^= number;
      if (result[i] == '\x00') {
        nullbyte = 1;
        fprintf(stderr, "Recode!\n");
        break;
      }
    }
  } while (nullbyte == 1);
  memcpy(shellcode, result, lshellcode);
  free(result);

  result = malloc(ldecoder + lshellcode + 1);
  memcpy(result, (const void *) decoder, ldecoder);
  memcpy(result + ldecoder, shellcode, lshellcode);
  *(result + ldecoder + lshellcode) = '\0';
  print_code("result", result, ldecoder + lshellcode);

  /* write2file */
  outfile = fopen(_OUTFILE, "w+b");
  if (outfile == NULL) err_n_xit("fopen", _OUTFILE);
  if (fwrite((void *) result, sizeof(char), strlen(result), outfile) != strlen(result)) err_n_xit("fwrite", _OUTFILE);
  if (fclose(outfile) != 0) err_n_xit("fclose", _OUTFILE);
  fprintf(stderr, "outfile: %s\n", _OUTFILE);

  free(result);
  return (0);
}
