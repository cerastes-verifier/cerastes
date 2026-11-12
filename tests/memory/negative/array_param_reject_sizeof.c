// expect: rejected cerastes-sizeof_array_argument @ L12
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (Step 0 M: both compilers warn -Wsizeof-array-argument without a flag): sizeof of an array parameter is the size of a pointer: rejected like the compilers, tag sizeof_array_argument
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int a[3])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return (int)sizeof(a);
}
