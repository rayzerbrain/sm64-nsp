#ifndef GFX_FIX_H
#define GFX_FIX_H

#include <stdint.h>

typedef int64_t fix64;

typedef union {
    fix64 n;
    struct { // little endian specific
        uint32_t d; // decimal
        int32_t i; // integer
    };
} fix64_s;

#define FRAC_WIDTH 32 // non configurable for mult and div operations

#define FIX_SPLIT(fix) ((fix64_s){fix})

#define FIX_ONE (1LL << FRAC_WIDTH)
#define FIX_ONE_HALF (1LL << (FRAC_WIDTH - 1))

#define FIX_2_FLOAT(fix) ((float) (fix) / (1LL << FRAC_WIDTH))
#define FLOAT_2_FIX(fl) ((fix64)((fl) * (1LL << FRAC_WIDTH))) // shifting float left FRAC_WIDTH bits

#define INT_2_FIX(num) ((fix64)(num) << FRAC_WIDTH) // all fixed point operations must involve two fix64 numbers
#define FIX_2_INT(fix) ((int)((fix) >> FRAC_WIDTH)) // non rounding

static inline fix64 fix_mult(const fix64 fix1, const fix64 fix2) {
    fix64_s s1 = FIX_SPLIT(fix1);
    fix64_s s2 = FIX_SPLIT(fix2);

    fix64_s result = (fix64_s){(int64_t)s1.i * s2.d 
                    + (int64_t)s2.i * s1.d
                    + ((uint64_t)s1.d * s2.d >> 32)}; // 
    result.i += s1.i * s2.i;

    /* float fresult = FIX_2_FLOAT(fix1) * FIX_2_FLOAT(fix2);

    if (FLOAT_2_FIX(fresult) != result.n && (s1.i == 0 && s2.i == 0)) {
        printf("%f, %lld, %lld\n", fresult, result.n, (uint64_t)fix1 * fix2);

        printf("%ld, %ld\n%lu, %lu\n", s1.i, s2.i, s1.d, s2.d);

        fix64_s f = FIX_SPLIT(FLOAT_2_FIX(fresult));

        printf("%ld, %lu\n%ld, %lu\n", result.i, result.d, f.i, f.d);
        abort();
    }*/

    return result.n;
}

static inline fix64 fix_div(const fix64 num, const fix64 denom) { //assumes non zero denominator
    float fnum = FIX_2_FLOAT(num);
    float fdenom = FIX_2_FLOAT(denom);

    return FLOAT_2_FIX(fnum / fdenom);
}


#endif