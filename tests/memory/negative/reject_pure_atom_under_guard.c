// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 items 1 and 3: an atom under a guard or || in a pure function's requires is rejected (M4.5f1): each atom is a top-level conjunct with no guard
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the atom's line)
#include <cerastes.h>

CERASTES_PURE int first(const int *a, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_reads_range(a, n))
{
  return n == 0 ? 0 : a[0];
}

int use(int y)
{
  return y;
}
