// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 7 (audit round 1): a comma ending in an enumerator is a value of its enumeration in C as in C++: c == (x++, GREEN) compares two values of one enumeration
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int eq_comma_green(enum color c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == ((int)c == 1 && x != -1))
{
  return c == (x++, GREEN) && x != 0;
}

int eq_comma_red_left(enum color c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == ((int)c == 0 && x != -1))
{
  return (x++, RED) == c && x != 0;
}
