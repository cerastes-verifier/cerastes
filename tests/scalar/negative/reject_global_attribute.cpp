// expect: rejected cerastes-unrecognized_attribute @ L6
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: an unrecognized attribute on a named constant is rejected, not ignored
#include <cerastes.h>

[[foo::bar]] const int K = 5;

int get()
{
  return K;
}
