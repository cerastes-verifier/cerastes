// expect: error postcondition @ L15
// expect: error postcondition @ L21
// expect: error postcondition @ L27
// expect: error postcondition @ L34
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_cond.c
// Pins: 0023 item 7: each comparison is exactly (int)c == <the truth value>, not (int)c != it
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int eq_true_false(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c != (x != 0)))
{
  return c == (x ? true : false);
}

int eq_cond_comparisons(enum color c, int x)
    CERASTES_ENSURES(r, r == ((int)c != (x != 0 ? x > 0 : x < 0)))
{
  return c == (x ? (x > 0) : (x < 0));
}

int eq_cond_bool(enum color c, bool b)
    CERASTES_ENSURES(r, r == ((int)c != (int)b))
{
  return c == (b ? b : b);
}

int eq_comma(enum color c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == ((int)c != (x + 1 > 0)))
{
  return c == (x++, x > 0);
}
