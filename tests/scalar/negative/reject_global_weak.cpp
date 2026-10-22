// expect: rejected cerastes-unsupported_attribute @ L7
// twin: tests/scalar/const_global.c
// Pins: 0017 items 1-2: a weak constant is rejected (extern makes it external in C++); the attribute form is refused
// by the attribute allow-list (0011 item 7) before the weak check
#include <cerastes.h>

[[gnu::weak]] extern const int W = 7;

int get()
{
  return W;
}
