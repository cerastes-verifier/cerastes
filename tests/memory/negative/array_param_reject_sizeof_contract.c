// expect: rejected cerastes-sizeof_array_argument @ L9
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (audit round 4, S2): sizeof(a) / sizeof(a[0]) of an array parameter as a range count in a requires is rejected like the same expression in the body (the compilers warn): sizeof_array_argument at the clause (one clause only, so one diagnostic)
// Pre-registered from record 0060 item 15 (audit round 4), before the code
#include <cerastes.h>

int f(int a[4])
    CERASTES_REQUIRES(cerastes_owns_range(a, sizeof(a) / sizeof(a[0])))
{
  (void)a;
  return 0;
}
