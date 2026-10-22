// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/enum_contract.c
// Pins: 0018 item 4: next(RED) returns RED, but the ensures says GREEN
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color next(enum color c)
    CERASTES_REQUIRES(c == RED || c == GREEN)
    CERASTES_ENSURES(r, (c == RED && r == GREEN) || (c == GREEN && r == BLUE))
{
  if (c == RED) {
    return RED;
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
