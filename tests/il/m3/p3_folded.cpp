// M3 probe P3: folded constants and the diagnostics EDG gives for them.
// expect: frontend-warning integer_overflow @ L18
// expect: frontend-warning shift_count_too_large @ L19
// expect: frontend-warning divide_by_zero @ L20
// expect: frontend-warning mod_by_zero @ L21
// expect: frontend-warning integer_truncated @ L22
// expect: frontend-warning integer_sign_change @ L23
// expect: frontend-warning integer_overflow @ L25
// expect: frontend-warning negative_shift_count @ L26
// expect: frontend-warning integer_sign_change @ L28
// expect: frontend-warning integer_overflow @ L29
// expect: frontend-warning integer_overflow @ L30
// expect: frontend-warning integer_overflow @ L31
// expect: frontend-warning integer_overflow @ L32
// expect: frontend-warning integer_truncated @ L37
// expect: frontend-warning integer_truncated @ L39
#include <limits.h>
int f_intmax() { return INT_MAX + 1; }
int f_shl40() { return 1 << 40; }
int f_div0() { return 1 / 0; }
int f_rem0() { return 1 % 0; }
signed char f_sc300() { signed char c = 300; return c; }
unsigned f_unsm1() { unsigned u = -1; return u; }
int f_short70000() { return (short)70000; }
int f_runtime(int x) { return x + (2147483647 + 1); }
int f_neg_shift() { return 1 << -1; }
int f_shr_neg() { return -8 >> 1; }
int f_umax() { return 0u - 1; }
long long f_ll() { return 9223372036854775807LL + 1; }
int f_imin() { return -2147483647 - 1 - 1; }
int f_mul() { return 65536 * 65536; }
int f_neg_int_min() { return -(-2147483647 - 1); }
// Implicitly converted constants: does the constant keep its expression?
long g_long() { long l = 5; return l; }
unsigned char g_uc() { unsigned char c = 255; return c; }
bool g_bool() { bool b = 1; return b; }
signed char g_ret() { return 300; }
void takes_sc(signed char);
void g_arg() { takes_sc(300); takes_sc(7); }
unsigned g_uarg(unsigned u) { return u + 1; }
int g_lt() { return 2147483647L < 5; }
long g_widen(int x) { return x + 5L; }
unsigned g_mixc(int x) { return x + 5u; }
