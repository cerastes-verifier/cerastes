// expect: error null-dereference @ L13
// mode: c23, c++20
// twin: tests/memory/null_conditional.c
// Pins: 0032 item 6: the ?: may give null and *p is not guarded
#include <stddef.h>
#include <cerastes.h>

int conditional(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 0))
{
  int x = 0;
  int *p = c != 0 ? &x : NULL;
  *p = 1;
  return x;
}
