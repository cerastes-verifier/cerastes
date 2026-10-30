// expect: rejected cerastes-return_local_address @ L11
// mode: c23, c++20
// twin: tests/memory/pick_owns.c
// Pins: 0039 item 6 (R14): return c ? &x : &y; is rejected at the return (both arms of ?: (clang warns, GCC does not; EDG's #1056 does not either))
#include <cerastes.h>

int *f(int c)
{
  int x = 1;
  int y = 2;
  return c ? &x : &y;
}
