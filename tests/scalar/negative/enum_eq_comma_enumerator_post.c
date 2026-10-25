// expect: error postcondition @ L14
// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/scalar/enum_eq_comma_enumerator.c
// Pins: 0023 item 7: the comparison with a comma ending in an enumerator is exact
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int eq_comma_green(enum color c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == ((int)c == 2 && x != -1))
{
  return c == (x++, GREEN) && x != 0;
}

int eq_comma_red_left(enum color c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == ((int)c == 1 && x != -1))
{
  return (x++, RED) == c && x != 0;
}
