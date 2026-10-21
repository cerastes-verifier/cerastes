// expect: rejected cerastes-unrecognized_attribute @ L8
// expect: rejected cerastes-call_without_contract @ L14
// twin: tests/scalar/old_cpp.cpp
// Pins: 0015 item 1: in C++20 [[_Noreturn]] is an unrecognized attribute that sets nothing, and is rejected as such
// The caller would otherwise be verified as if f returned.
#include <cerastes.h>

[[_Noreturn]] void f(void);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    f();
  }
  return x;
}
