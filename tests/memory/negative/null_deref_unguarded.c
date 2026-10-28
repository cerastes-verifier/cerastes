// expect: error null-dereference @ L16
// mode: c23, c++20
// twin: tests/memory/null_guard.c
// Pins: 0032 item 6: the guard is gone and *p is applied to a null pointer when c == 0
#include <stddef.h>
#include <cerastes.h>

int guarded(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 7 : 0))
{
  int x = 0;
  int *p = NULL;
  if (c != 0) {
    p = &x;
  }
  *p = 7;
  return x;
}
