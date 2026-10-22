// expect: frontend-warning integer_overflow @ L8
// expect: frontend-warning integer_overflow @ L9
// M3 probe P9, the overflow side (M4.2c, pin f0ed5525af): since upstream #38 the front end folds __int128 constants with
// 128-bit limits, so a product that does not fit __int128 is warning #61, and one that fits is not (the first two
// functions: 2^65 and 3 * 2^125, warned before #38); an unsigned __int128 product wraps without a warning.
__int128 fits_2_pow_65() { return (__int128)9223372036854775807LL * 4; }
__int128 fits_3_pow_125() { return ((__int128)1 << 125) * 3; }
__int128 overflows_2_pow_127() { return ((__int128)1 << 125) * 4; }
__int128 overflows_2_pow_128() { return ((__int128)1 << 126) * 4; }
unsigned __int128 wraps_2_pow_128() { return ((unsigned __int128)1 << 127) * 2; }
