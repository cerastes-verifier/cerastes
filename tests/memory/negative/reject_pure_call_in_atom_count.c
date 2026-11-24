// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 4: a call in the count of an atom is rejected (the positions that must read no memory: a call counts as a read; M4.5e2), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int cnt(int x)
{
  return x;
}

int use(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, cnt(n)))
{
  (void)a;
  return n;
}
