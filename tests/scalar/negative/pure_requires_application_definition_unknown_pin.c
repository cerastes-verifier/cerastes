// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/scalar/pure_calls_pure_in_requires.c
// Pins: 0062 item 8's listed limit (a false alarm, fail closed; M4.8, question 279): the definition of a pure function that a requires applies is not known in the definition check, so pred's x - 1 may overflow although positive(x) means 0 < x; the pair pure_calls_pure_in_requires.c gives positive an ensures r == (0 < x), which fact (ii) turns into the bound (question 282)
// Pre-registered from record 0062 (Part B), before the code: overflow (the pinned false alarm). the original program of that pair, measured by the first run of Part B (build/m4.5e1d/tm2/run-b.log)
#include <cerastes.h>

CERASTES_PURE bool positive(int x)
{
  return 0 < x;
}

CERASTES_PURE int pred(int x)
    CERASTES_REQUIRES(positive(x))
{
  return x - 1;
}

int use(int y)
    CERASTES_REQUIRES(0 < y)
{
  CERASTES_CHECK(pred(y) == pred(y));
  return y;
}
