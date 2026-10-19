// expect: rejected undefined_identifier @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0011 item 8: a program EDG rejects is rejected with EDG's diagnostic (exit 2)
#include <cerastes.h>

int broken(int x)
{
  return zzz;
}
