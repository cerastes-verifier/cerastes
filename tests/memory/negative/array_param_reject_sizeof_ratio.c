// expect: rejected cerastes-sizeof_array_argument @ L12
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15: the classic element count sizeof(a) / sizeof(a[0]) of an array parameter is rejected (clang also warns -Wsizeof-pointer-div)
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int f(int a[3])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return (int)(sizeof(a) / sizeof(a[0]));
}
