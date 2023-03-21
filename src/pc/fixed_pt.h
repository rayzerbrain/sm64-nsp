#ifndef FIXED_PT_H
#define FIXED_PT_H

#include <stdint.h>

typedef int64_t fix64;

#define FRAC_WIDTH 32 // non configurable for mult

#define GET_INT(fix) (int32_t)((fix) >> 32)
#define GET_FRAC(fix) (uint32_t)((fix) & 0xffffffff)

#define FIX_ONE (1LL << FRAC_WIDTH)
#define FIX_ONE_HALF (1LL << (FRAC_WIDTH - 1))

#define FIX_2_INT(fix) (((fix) >> FRAC_WIDTH)) // non rounding
#define FIX_2_FLOAT(fix) ((float) (fix) / (1LL << FRAC_WIDTH))
#define FIX_2_DOUBLE(fix) ((double) (fix) / (1LL << FRAC_WIDTH))

#define INT_2_FIX(num) ((fix64)(num) << FRAC_WIDTH) // all fixed point operations must involve two fix64 numbers
#define FLOAT_2_FIX(f) ((fix64)((f) * (1LL << FRAC_WIDTH))) // shifting float left FRAC_WIDTH bits
#define DOUBLE_2_FIX(d) ((fix64) ((d) * (1LL << FRAC_WIDTH))) // same as above

#define FIX_INV(fix) ((1ULL << 63) / (fix) << 1) // only loses one bit of precision (?)

static inline fix64 fix_mult(const fix64 fix1, const fix64 fix2) { // multiply 2 fixes, return fix
    fix64 i1 = GET_INT(fix1);
    fix64 f1 = GET_FRAC(fix1);
    fix64 i2 = GET_INT(fix2);
    fix64 f2 = GET_FRAC(fix2);
    
    return (((uint64_t) f1 * f2) >> 32) 
                    + ((int64_t)(i1 * i2) << 32)
                    + (int64_t)i1 * f2
                    + (int64_t)i2 * f1;
}

static inline int32_t fix_mult_i(const fix64 fix1, const fix64 fix2) { // multiply two fixes, return integer part. saves time not calculating decimal * decimal
    fix64 i1 = GET_INT(fix1);
    fix64 f1 = GET_FRAC(fix1);
    fix64 i2 = GET_INT(fix2);
    fix64 f2 = GET_FRAC(fix2);
    
    return (int32_t) ( ((int64_t) i1 * f2 + (int64_t) i2 * f1) >> 32 ) 
        + (int32_t)i1 * (int32_t)i2;
}

// assumes non zero denominators
static inline fix64 fix_div_s(const fix64 num, const fix64 denom) { // much easier than finding fast integer division (?)
    return (fix64)((float)num / FIX_2_FLOAT(denom));
}

static inline fix64 fix_div_d(const fix64 num, const fix64 denom) {
    return (fix64)((double)num / FIX_2_DOUBLE(denom));
}


#endif