// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23
// c-only: restrict does not exist in C++20 (EDG, clang++ and g++ refuse it, decision 0037 item 2)
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 item 1 (0037 item 2): restrict on a read pure function's parameter is rejected with the tag it has on any function (M4.5f1); forall_reject_f_restrict.c is the same rejection at a forall
// Pre-registered from record 0062 (Part B), before the code: rejected. C only (restrict is not C++); the tag and the line are question-160 candidates (the function's own line)
#include <cerastes.h>

CERASTES_PURE int first(const int *restrict a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_reads_range(a, n))
{
  return a[0];
}

int use(int y)
{
  return y;
}
