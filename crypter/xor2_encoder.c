//	#DECODER=./xor2_decoder.o
//	#SHELLCODE=../shellcode/hello.o
#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <byteswap.h>

#ifdef _USE_CFG
#include "xor2_encoder.h"
#else
#error "xor2_encoder.h config file missing including decoder && shellcode"
#endif

#define XOR_KEYLEN 5
#define SCLEN_XORKEY 0x0101
#define TRAILER 3

#ifndef _OUTFILE
#define _OUTFILE "xor2_encoded.o"
#endif


long int
getnumber(long int n)
{
  int seed;
  struct timeval tm;

  gettimeofday(&tm, NULL);
  seed = tm.tv_sec + tm.tv_usec;
  srandom(seed);
  return (random() % n);
}

void
print_code(const char *name, char *data, size_t len)
{
  int i,l = 15;

  printf("unsigned long int l%s = %lu;\nchar %s[] = \n", name, (unsigned long int) len, name);
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

long int
eof_check(char *data, size_t len)
{
  long int i;

  for (i = 0; i < len; i++) {
    if ( *(char *)(data + i) == '\0' ) {
      return i;
    }
  }
  return -1;
}

char *
xor_genkey(size_t keylen)
{
  char *key;
  long int kd, rnd;
  int i = 0;

  key = calloc(sizeof(char), keylen);
  while (i+sizeof(long int) < keylen) {
    rnd = getnumber(LONG_MAX);
    memcpy(&key[i], &rnd, sizeof(long int));
    i += sizeof(long int);
  }
  kd = keylen - i;
  if ( kd != 0 ) {
    rnd = getnumber(LONG_MAX);
    memcpy(&key[i], &rnd, kd);
  }
  return key;
}

void
xor_encrypt(char *buf, size_t buflen, char *key, size_t keylen)
{
  int i;
  unsigned char xb;

  for (i = 0; i < buflen; i++) {
    xb = key[i % keylen];
    buf[i] ^= xb;
  }
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
  int nullbyte = 0;
  long int nb_idx;
  int ldecoder = sizeof(decoder)-1; /* last byte is '\x00' */
  uint16_t lshellcode = (uint16_t) sizeof(shellcode)-1; /* same as above */
  char *result, *mod_decoder, *xor_key;
  FILE *outfile;

  printf("/* PRINT SHELLCODE */\n");
  print_code("shellcode", shellcode, lshellcode);
  printf("/* PRINT DECODER */\n");
  print_code("decoder", decoder, ldecoder);

  mod_decoder = malloc(ldecoder + TRAILER);  // buffer size (2 bytes) + xor key len (1 byte)
  memcpy(mod_decoder, decoder, ldecoder);
  *(uint16_t *) (&mod_decoder[ldecoder]) = (uint16_t) (lshellcode ^ SCLEN_XORKEY);
  *(uint8_t *) (&mod_decoder[ldecoder+2]) = (uint8_t) XOR_KEYLEN;
  printf("/* shellcode length: decoder[%u] = %u bytes ^ 0x%04x = 0x%04x */\n", lshellcode, mod_decoder[ldecoder], SCLEN_XORKEY, *(uint16_t *) &mod_decoder[ldecoder]);
  printf("/* xor key length: decoder[%u] = %u bytes = 0x%02x */\n", ldecoder+2, mod_decoder[ldecoder+2], mod_decoder[ldecoder+2]);

  if ( (nb_idx = eof_check(mod_decoder , ldecoder+3)) != -1) {
    printf("NULLBYTE DETECTED: decoder+0x%04x (%lu)\n", (unsigned int) nb_idx, nb_idx);
    exit(-1);
  }

  result = calloc(ldecoder + lshellcode + TRAILER + XOR_KEYLEN, sizeof(char));
  printf("/* total length = %d */\n", ldecoder + lshellcode + TRAILER + XOR_KEYLEN);
  memcpy(result, mod_decoder, ldecoder + TRAILER);
  free(mod_decoder);
  do {
    xor_key = xor_genkey(XOR_KEYLEN);
    memcpy(result + ldecoder + TRAILER, xor_key, XOR_KEYLEN);
    memcpy(result + ldecoder + TRAILER + XOR_KEYLEN, shellcode, lshellcode);
    xor_encrypt(result + ldecoder + TRAILER + XOR_KEYLEN, lshellcode, xor_key, XOR_KEYLEN);
    print_code("xor", xor_key, XOR_KEYLEN);

    if (nullbyte == 1) {
      nullbyte = 0;
    }

    free(xor_key);
  } while (nullbyte == 1);

  print_code("result", result, ldecoder + lshellcode + TRAILER + XOR_KEYLEN);

  /* write2file */
  outfile = fopen(_OUTFILE, "w+b");
  if (outfile == NULL) err_n_xit("fopen", _OUTFILE);
  if (fwrite((void *) result, sizeof(char), ldecoder + lshellcode + TRAILER + XOR_KEYLEN, outfile) != (ldecoder + lshellcode + TRAILER + XOR_KEYLEN)) err_n_xit("fwrite", _OUTFILE);
  if (fclose(outfile) != 0) err_n_xit("fclose", _OUTFILE);
  fprintf(stderr, "outfile: %s\n", _OUTFILE);

  free(result);
  return (0);
}
