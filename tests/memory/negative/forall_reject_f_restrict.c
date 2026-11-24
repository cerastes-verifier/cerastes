// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (audit round 1 S13, 0037 items 2 and 15): a restrict parameter of f is rejected with the tag it has on a function with no contract
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool restricted(int i, const int *restrict b, int m)
{
  return !(0 <= i && i < m) || b[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(restricted, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
