// expect: rejected expr_not_constant @ L6
// mode: c++20
// twin: tests/scalar/enum_init.c
// Pins: 0019 item 9 (EDG bug 0022, fixed in the pin): an enumerator 10 / 0 is not a constant (error #28, note #39 suppressed)
#include <cerastes.h>
enum { A = 10 / 0 };

int f(void)
{
  return A;
}
