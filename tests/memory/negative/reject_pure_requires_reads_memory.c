// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: a value clause of a pure function's requires that reads memory is rejected (M4.5f1), at the clause
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int deref(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p > 0)
{
  return *p;
}

int use(void)
{
  int x = 1;
  CERASTES_CHECK(deref(&x) == deref(&x));
  return x;
}
