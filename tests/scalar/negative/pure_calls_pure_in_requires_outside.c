// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/scalar/pure_calls_pure_in_requires.c
// Pins: 0062 items 3, 4 and 7 (cut 4): applying a pure function whose requires applies another one outside it: the precondition error at the check
// Pre-registered from record 0062 (Part B), before the code: precondition. First run (build/m4.5e1d/tm2/run-b.log): the function `pred` overflowed at `x - 1`, because its `requires positive(x)` is an application whose definition is not known in the definition check (record 0062 item 8, listed limit): `positive` now has the ensures `r == (0 < x)`, which fact (ii) gives the check; the line moved by one
#include <cerastes.h>

CERASTES_PURE bool positive(int x)
    CERASTES_ENSURES(r, r == (0 < x))
{
  return 0 < x;
}

CERASTES_PURE int pred(int x)
    CERASTES_REQUIRES(positive(x))
{
  return x - 1;
}

int use(int y)
    CERASTES_REQUIRES(-5 < y)
{
  CERASTES_CHECK(pred(y) == pred(y));
  return y;
}
