// expect: error permission @ L11
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/range_guards.c
// Pins: 0060 item 3 (audit B1) and item 9 (S1), a function with a body: the requires range is under n == 0 || and the ensures range is not. f__wf: permission at the ensures atom (it returns p[0..n) on a path where its requires did not take it); then on the n == 0 path one permission at the return (the row is named in report 7); the bodiless form needs a call: Part C
// Pre-registered from record 0060 items 3 and 9, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  return 0;
}
