// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 1: unsigned + - * and unary minus whose mathematical result fits the type are exact, with no error
#include <cerastes.h>

unsigned sub_u(unsigned a)
    CERASTES_REQUIRES(a == 1u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a - 1u;
}

unsigned add_u(unsigned a)
    CERASTES_REQUIRES(a == 4294967294u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return a + 1u;
}

unsigned mul_u(unsigned a)
    CERASTES_REQUIRES(a == 65535u)
    CERASTES_ENSURES(r, r == 4294901760u)
{
  return a * 65536u;
}

unsigned neg_u(unsigned a)
    CERASTES_REQUIRES(a == 0u)
    CERASTES_ENSURES(r, r == 0u)
{
  return -a;
}
