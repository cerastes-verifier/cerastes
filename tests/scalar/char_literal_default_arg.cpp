// expect: verified
// expect: native-ok
// Pins: 0024 item 5: a default argument's literal is checked where the default is used (0012 item 6)
#include <cerastes.h>

int pick(int a, char b = '\x7f')
    CERASTES_REQUIRES(a >= -2147483520 && a <= 2147483520)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

int use(void)
    CERASTES_ENSURES(r, r == 127)
{
  return pick(0);
}

int use_edge(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return pick(2147483520);
}

int use_low(void)
    CERASTES_ENSURES(r, r == -2147483520)
{
  return pick(-2147483520, 0);
}

int use_explicit(void)
    CERASTES_ENSURES(r, r == 11)
{
  return pick(1, 10);
}
