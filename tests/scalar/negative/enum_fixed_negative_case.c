// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/scalar/enum_fixed_negative.c
// Pins: 0024 item 5: enum_fixed_negative.c with the first arm returning 11 under a postcondition that says 10
#include <cerastes.h>

enum EI : int { IM = -2, IP = 3 };

enum EI minus_two(void)
    CERASTES_ENSURES(r, (int)r == -2)
{
  enum EI x = IM;
  return x;
}

int pick(enum EI e)
    CERASTES_ENSURES(r, (e == IM && r == 10) || (e == IP && r == 20) || (e != IM && e != IP && r == 0))
{
  switch (e) {
  case IM:
    return 11;
  case IP:
    return 20;
  default:
    return 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 30)
{
  return pick(IM) + pick(IP) + pick(minus_two()) - 10;
}
