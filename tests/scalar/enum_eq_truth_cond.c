// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 7 (Q36, amends 0022 item 5): == between an enumeration and a ?: whose arms are truth values, or a comma whose right operand is one, compares the values in both modes (0 or 1, never negative)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int eq_true_false(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c == (x != 0)))
{
  return c == (x ? true : false);
}

int eq_cond_comparisons(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c == (x != 0 ? x > 0 : x < 0)))
{
  return c == (x ? (x > 0) : (x < 0));
}

int eq_cond_bool(enum color c, bool b)
    CERASTES_ENSURES(r, r == ((int)c == (int)b))
{
  return c == (b ? b : b);
}

int eq_comma(enum color c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == ((int)c == (x + 1 > 0)))
{
  return c == (x++, x > 0);
}
