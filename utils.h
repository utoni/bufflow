#ifndef UTILS_H
#define UTILS_H 1

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLEN 961 // maximum string length

static inline size_t makehist(unsigned char const * const buf, ssize_t * const hist, size_t len) {
  ssize_t wherechar[256];
  size_t i, histlen;
  histlen = 0;
  for (i = 0; i < 256; i++)
    wherechar[i] = -1;
  for (i = 0; i < len; i++) {
    if (wherechar[buf[i]] == -1) {
      wherechar[(int)buf[i]] = histlen;
      histlen++;
    }
    hist[wherechar[(int)buf[i]]]++;
  }
  return histlen;
}

static inline double entropy(ssize_t * const hist, size_t histlen, size_t len) {
  size_t i;
  double H;
  H = 0.0;
  for (i = 0; i < histlen; i++) {
    H -= (double)hist[i] / len * log2((double)hist[i] / len);
  }
  return H;
}

static inline double entropy_from_buffer(unsigned char const * const buffer, size_t size)
{
  ssize_t * const hist_array = malloc(size * sizeof(*hist_array));

  if (!hist_array) {
    return -1.0;
  }

  size_t hist_length = makehist(buffer, hist_array, size);
  double entr = entropy(hist_array, hist_length, size);

  free(hist_array);
  return entr;
}

#endif
