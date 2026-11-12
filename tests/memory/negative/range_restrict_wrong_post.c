// expect: error postcondition @ L16
// mode: c23
// c-only: restrict does not exist in C++20 (decision 0037 item 2)
// twin: tests/memory/range_restrict.c
// Pins: 0060 item 14: set0 claims p[0] == 1 where it stored 0: postcondition at the closing brace
// Pre-registered from record 0060 item 14 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void set0(int *restrict p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 1)
{
  (void)n;
  p[0] = 0;
}

void set0_guarded(int *restrict p, int n)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(p == NULL || cerastes_owns_range(p, n))
{
  if (p != NULL && n > 0) {
    p[0] = 0;
  }
}

void two(int *restrict dst, int *restrict src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_owns_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_owns_range(src, n))
{
  (void)dst;
  (void)src;
  (void)n;
}
