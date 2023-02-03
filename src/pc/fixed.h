#ifndef FIXED_H
#define FIXED_H

#ifdef GBI_FLOATS
typedef float s32real;
#else
typedef int32_t s32real;

#define FIX_2_INT(fix) ((fix) >> 16)       // non-rounding
#define FIX_2_FLOAT(fix) ((fix) / 65536.f) // 2^16
#define INT_2_FIX(num) ((num) << 16)
#define FLOAT_2_FIX(fl) ((int32_t) ((fl) *65536)) // 2^16

#define FIX_ADD_INT(fix, num) ((fix) + (num) >> 16)
#define FIX_MUL(fix1, fix2) ((int32_t) (((int64_t) (fix1) * (fix2)) >> 16))
#define FIX_MUL_FLOAT(fix, fl) ((int32_t) ((fix) * (fl)))
#define FIX_DIV_FLOAT(fix, fl) ((int32_t) ((fix) / (fl))) // all of these return fix types
#endif

#endif