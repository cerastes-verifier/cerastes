// expect: rejected cerastes-unsupported_expression @ L20
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: STAGING PIN S1, deleted by Part C: a call of a function whose contract holds a range atom is rejected until the caller's rows exist (record 0060 item 8; the encoder's tag: unsupported_expression at the call)
// Pre-registered from record 0060 item 8 (staging pin S1), before the code
#include <cerastes.h>

int get(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[i])
{
  (void)n;
  return p[i];
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  return get(a, 3, 0);
}
