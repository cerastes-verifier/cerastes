// expect: rejected cerastes-array_parameter_mismatch @ L10
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (Step 0 M: -Warray-parameter at -Wall in clang, clang++ and GCC's C): two declarations with different bounds: rejected in both modes (the union rule), at the later declaration
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int a[3]);

int f(int a[4])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[0];
}
