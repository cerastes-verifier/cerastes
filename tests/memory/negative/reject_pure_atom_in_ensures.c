// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 items 1 and 3: any atom in a pure function's ensures is rejected (M4.5f1): the function takes nothing and returns nothing, it reads
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the atom's line)
#include <cerastes.h>

CERASTES_PURE int first(const int *a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[0])
{
  return a[0];
}

int use(int y)
{
  return y;
}
