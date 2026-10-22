// expect: rejected expr_not_constant @ L6
// mode: c++20
// twin: tests/scalar/const_global_init_named.cpp
// Pins: 0019 item 9 (EDG bug 0022, fixed in the pin): a constexpr global 1 << 32 is not a constant (error #28, note #63 suppressed)
#include <cerastes.h>
constexpr int z = 1 << 32;

int f()
{
  return z;
}
