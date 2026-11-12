// expect: rejected cerastes-sizeof_array_argument @ L10
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (audit round 4, S2): sizeof(a) == 8 of an array parameter in an ensures (the size of a pointer, folded by the front end) is rejected: sizeof_array_argument at the clause
// Pre-registered from record 0060 item 15 (audit round 4), before the code
#include <cerastes.h>

int f(int a[4])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a) && sizeof(a) == 8)
{
  return a[0];
}
