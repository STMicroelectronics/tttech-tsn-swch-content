#include "bmp.h"

void bmp_set_bit(bmp_word_t *bmp, int n) {
    bmp[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void bmp_clear_bit(bmp_word_t *bmp, int n) {
    bmp[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

int bmp_get_bit(bmp_word_t *bmp, int n) {
    bmp_word_t bit = bmp[WORD_OFFSET(n)] & (1U << BIT_OFFSET(n));
    return 0 != bit;
}