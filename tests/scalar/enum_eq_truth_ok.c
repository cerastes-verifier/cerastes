// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 5 (owner, 2026-10-23): == between an enumeration and a truth-valued int (a comparison, !, &&) compares the values: the operand is 0 or 1, never negative
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum neg { NA = -1, NB };

int eq_cmp(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c == (x > 0)))
{
  return c == (x > 0);
}

int eq_not(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c == !x))
{
  return c == !x;
}

int eq_and(enum color c, int x, int y)
    CERASTES_ENSURES(r, r == ((int)c == (x && y)))
{
  return c == (x && y);
}

int eq_zero(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c == (x == 0)))
{
  return c == (x == 0);
}

int eq_enums(enum color c, enum neg n)
    CERASTES_ENSURES(r, r == ((int)c == (int)n))
{
  return (int)c == (int)n;
}
