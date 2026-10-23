// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 7: a modular result is written as a reduction in a wider type; the conversion back is then exact
#include <cerastes.h>

unsigned char low_byte(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x % 256)
{
  return (unsigned char)(x % 256);
}

unsigned hash_step(unsigned h, unsigned k)
    CERASTES_REQUIRES(h == 4294967295u && k == 2u)
    CERASTES_ENSURES(r, r == 4294967294u)
{
  return (unsigned)(((unsigned long long)h * k) % 4294967296ull);
}

unsigned hash_run(void)
    CERASTES_ENSURES(r, r == 4294967294u)
{
  return hash_step(4294967295u, 2u);
}
