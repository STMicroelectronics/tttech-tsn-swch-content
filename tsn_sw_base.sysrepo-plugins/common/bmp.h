#ifndef _BMP_H__
#define _BMP_H__

#include <limits.h>

typedef int bmp_word_t;

enum { BITS_PER_WORD = sizeof(bmp_word_t) * CHAR_BIT };

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

void bmp_set_bit(bmp_word_t *bmp, int n);

void bmp_clear_bit(bmp_word_t *bmp, int n);

int bmp_get_bit(bmp_word_t *bmp, int n);

#endif
