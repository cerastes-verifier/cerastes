// expect: error postcondition @ L40
// mode: c23, c++20
// twin: tests/scalar/bit_long.c
// Pins: 0023 decision: 2^62 & (2^63 - 1) is 2^62, not one less
#include <cerastes.h>

long and_long(long a, long b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

unsigned long or_ulong(unsigned long a, unsigned long b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

long long xor_ll(long long a, long long b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

long long not_ll(long long a)
    CERASTES_ENSURES(r, r == -1 - a)
{
  return ~a;
}

unsigned long long not_ull(unsigned long long a)
    CERASTES_ENSURES(r, r == 18446744073709551615ull - a)
{
  return ~a;
}

long and_long_value(void)
    CERASTES_ENSURES(r, r == 4611686018427387903L)
{
  return and_long(4611686018427387904L, 9223372036854775807L);
}

unsigned long or_ulong_value(void)
    CERASTES_ENSURES(r, r == 9223372036854775809ul)
{
  return or_ulong(9223372036854775808ul, 1ul);
}

long long xor_ll_value(void)
    CERASTES_ENSURES(r, r == -9223372036854775807LL - 1)
{
  return xor_ll(-1LL, 9223372036854775807LL);
}

long long not_ll_min(void)
    CERASTES_ENSURES(r, r == 9223372036854775807LL)
{
  return not_ll(-9223372036854775807LL - 1);
}

unsigned long long not_ull_zero(void)
    CERASTES_ENSURES(r, r == 18446744073709551615ull)
{
  return not_ull(0ull);
}
