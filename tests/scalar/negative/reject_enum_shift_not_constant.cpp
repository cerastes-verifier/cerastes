// expect: rejected expr_not_constant @ L7
// mode: c++20
// twin: tests/scalar/enum_init.c
// Pins: 0019 item 9 (EDG bug 0022, fixed in the pin): in C++ an enumerator 1 << 32 is not a constant (error #28); its note
// #63 is suppressed in verify mode, so the error comes alone (exit 2), not internal error #2656. C: enum_init_shift.c
#include <cerastes.h>
enum { A = 1 << 32 };

int f(void)
{
  return A;
}
