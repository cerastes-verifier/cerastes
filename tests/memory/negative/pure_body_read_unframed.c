// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 item 9: a read in a pure body that the function's atoms do not frame (the second element of a one-element atom) is the permission error, in the function, at the read
// Pre-registered from record 0062 (Part B), before the code: permission
#include <cerastes.h>

CERASTES_PURE int second(const int *a)
    CERASTES_REQUIRES(cerastes_reads_range(a, 1))
{
  return a[1];
}

int use(void)
{
  int a[2] = {1, 2};
  CERASTES_CHECK(second(a) == second(a));
  return a[0];
}
