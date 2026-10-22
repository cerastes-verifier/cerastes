// expect: rejected cerastes-unsupported_attribute @ L8
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 items 1-2: a weak constant may be replaced at link time; the attribute form is refused by the attribute
// allow-list (0011 item 7) before the weak check (which `#pragma weak` reaches: reject_global_weak_pragma.c)
#include <cerastes.h>

extern const int W __attribute__((weak)) = 7;

int get(void)
{
  return W;
}
