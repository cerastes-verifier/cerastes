// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c++20
// twin: tests/memory/static_local_unused.c
// Pins: 0041 item 2 (audit N7): a C++ static local initialized from a local constant: its initializer names a local, which the prologue (the initializer's closed form) cannot read: rejected
#include <cerastes.h>

int get()
{
  const int k = 5;
  static int s = k;
  return s;
}
