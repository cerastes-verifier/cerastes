// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_explicit_mod.c
// Pins: 0022 item 7: (2^32 - 1) * 2 mod 2^32 is 4294967294u, not 4294967295u
#include <cerastes.h>

unsigned hash_step(unsigned h, unsigned k)
    CERASTES_REQUIRES(h == 4294967295u && k == 2u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return (unsigned)(((unsigned long long)h * k) % 4294967296ull);
}
