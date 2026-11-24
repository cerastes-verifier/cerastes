// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 items 1 and 3 (M4.5f1): a cerastes_forall in the body of a read pure function (a named quantified property) is unsupported_annotation (the tag a forall inside E has, forall_reject_nested.c)
// Pre-registered from record 0062 (Part B), before the code: rejected. the tag and the line are question-160 candidates (the construct's own line: the function is read by its requires alone)
#include <cerastes.h>

CERASTES_PURE bool nonneg_at(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

CERASTES_PURE bool all_nonneg(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
{
  return cerastes_forall(nonneg_at, a, n);
}

int use(int y)
{
  return y;
}
