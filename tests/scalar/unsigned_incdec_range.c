// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 items 1 and 4: ++, --, += -= *= on unsigned int reach the boundary values exactly; the literal 1 of u++ and u += 1 needs no check
#include <cerastes.h>

unsigned inc(unsigned u)
    CERASTES_REQUIRES(u == 4294967294u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  u++;
  return u;
}

unsigned dec(unsigned u)
    CERASTES_REQUIRES(u == 1u)
    CERASTES_ENSURES(r, r == 0u)
{
  u--;
  return u;
}

unsigned add_k(unsigned u, unsigned k)
    CERASTES_REQUIRES(u == 4294967290u && k == 5u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  u += k;
  return u;
}

unsigned sub_k(unsigned u, unsigned k)
    CERASTES_REQUIRES(u == 3u && k == 3u)
    CERASTES_ENSURES(r, r == 0u)
{
  u -= k;
  return u;
}

unsigned mul_k(unsigned u, unsigned k)
    CERASTES_REQUIRES(u == 65535u && k == 65537u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  u *= k;
  return u;
}

unsigned plus_one(unsigned u)
    CERASTES_REQUIRES(u <= 4294967294u)
    CERASTES_ENSURES(r, r == u + 1u)
{
  u += 1;
  return u;
}

unsigned plus_one_top(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return plus_one(4294967294u);
}
