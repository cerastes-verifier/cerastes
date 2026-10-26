// expect: rejected cerastes-unsupported_type @ L10
// (the use of y at line 11 is not reported again: a rejected declared type is reported once, at the declaration)
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6 (round 1 audit): the rejected type is reported once, at the declaration, and not again at the use of y; a local whose type is `std::conditional_t<(65536u * 65536u == 0u), int, long>` reaches its meaning through an alias-template instance with a non-type argument
#include <cerastes.h>
#include <type_traits>

int widen(int x)
{
  std::conditional_t<(65536u * 65536u == 0u), int, long> y = x;
  return (int)y;
}
