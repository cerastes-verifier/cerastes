// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 5: the by-value-and-type rule leaves enumerators of a fixed enumeration of rank at least int alone (a C23 `case IM:` and a copy keep today's reading)
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
    return 10;
  case IP:
    return 20;
  default:
    return 0;
  }
}

int other(void)
    CERASTES_ENSURES(r, r == 0)
{
  return pick((enum EI)0);
}

int use(void)
    CERASTES_ENSURES(r, r == 30)
{
  return pick(IM) + pick(IP) + pick(minus_two()) - 10;
}
