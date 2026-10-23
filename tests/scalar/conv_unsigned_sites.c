// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 1: a conversion into an unsigned type whose value fits is exact, at an initializer, an assignment, a return, a cast, an argument, the usual conversions and a compound assignment
#include <cerastes.h>

unsigned from_init(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (unsigned)x)
{
  unsigned u = x;
  return u;
}

unsigned from_assign(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (unsigned)x)
{
  unsigned u;
  u = x;
  return u;
}

unsigned from_return(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (unsigned)x)
{
  return x;
}

unsigned from_cast(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (unsigned)x)
{
  return (unsigned)x;
}

unsigned take(unsigned a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

unsigned from_arg(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (unsigned)x)
{
  return take(x);
}

unsigned from_add(int i, unsigned u)
    CERASTES_REQUIRES(0 <= i && u <= 4294967295u - i)
    CERASTES_ENSURES(r, r == (unsigned)i + u)
{
  return i + u;
}

unsigned from_compound(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (unsigned)x + 5u)
{
  unsigned u = 5u;
  u += x;
  return u;
}

unsigned char to_uc(int x)
    CERASTES_REQUIRES(0 <= x && x <= 255)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int above_zero(int i)
    CERASTES_REQUIRES(0 <= i)
    CERASTES_ENSURES(r, r == (i > 0))
{
  return i + 0u > 0u;
}

unsigned edge_zero(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return from_add(0, 4294967295u);
}

unsigned edge_top(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return from_add(5, 4294967290u);
}
