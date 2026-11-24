// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 3 and 4 (cut 4): a pure function whose requires applies another pure function
// Pre-registered from record 0062 (Part B), before the code: verifies. First run (build/m4.5e1d/tm2/run-b.log): the function `pred` overflowed at `x - 1`, because its `requires positive(x)` is an application whose definition is not known in the definition check (record 0062 item 8, listed limit): `positive` now has the ensures `r == (0 < x)`, which fact (ii) gives the check; the line moved by one. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one
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
    CERASTES_REQUIRES(0 < y)
{
  CERASTES_CHECK(pred(y) == y - 1);
  return y;
}
