#ifndef GFX_FIX_H
#define GFX_FIX_H

#include <stdint.h>
#include <string.h>
#include <gmp.h>

typedef int64_t fix64; // 32 bit int and fraction part, signed

#define FRAC_WIDTH 32 // non configurable

#define FIX_2_FLOAT(fix) ((float) (fix) / (1LL << FRAC_WIDTH))
#define FLOAT_2_FIX(fl) ((fix64)((fl) * (1LL << FRAC_WIDTH))) // shifting float left FRAC_WIDTH bits

#define INT_2_FIX(num) ((fix64)(num) << FRAC_WIDTH) // all fixed point operations must involve two fix64 numbers
#define FIX_2_INT(fix) ((int)((fix64) >> FRAC_WIDTH) // non rounding

static inline fix64 fix_mult(const fix64 fix1, const fix64 fix2) {
    static uint32_t result[4];

    mpn_mul_n(result, (uint32_t *) &fix1, (uint32_t *) &fix2, 2);

    return *(fix64 *) &result[1];
}

static inline fix64 fix_div(const fix64 num, const fix64 denom) { //assumes non zero denominator
    static uint32_t num_padded[4], quo[3], rem[2];

    memcpy((void *) num_padded + 2 * sizeof(void *), &num, 2 * sizeof(void *)); // construct a fake 128 bit num with 64 least significant zeroes

    mpn_tdiv_qr(quo, rem, 0, num_padded, 4, (uint32_t*) &denom, 2); // divide num_padded (4 limbs) by denom (2 limbs). remainder is unneeded but must be stored regardless
    
    return *(fix64 *) &quo[1]; 
}


#endif