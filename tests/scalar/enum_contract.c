// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 items 4 and 9: enumerators in requires and ensures, an enumeration result and argument, calls that pass enumerators on
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color next(enum color c)
    CERASTES_REQUIRES(c == RED || c == GREEN)
    CERASTES_ENSURES(r, (c == RED && r == GREEN) || (c == GREEN && r == BLUE))
{
  if (c == RED) {
    return GREEN;
  }
  return BLUE;
}

enum color after_red(void)
    CERASTES_ENSURES(r, r == BLUE)
{
  return next(next(RED));
}

enum color step(enum color c)
    CERASTES_REQUIRES(c == RED)
    CERASTES_ENSURES(r, r == BLUE)
{
  return next(next(c));
}

enum color start(void)
    CERASTES_ENSURES(r, r == BLUE)
{
  return step(RED);
}
