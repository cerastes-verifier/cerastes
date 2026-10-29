// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/consume_owned.c
// Pins: 0037 items 11 and 13: a later read of the cell by name fails permission (a callee can keep a cell, so this is the user's error)
#include <cerastes.h>
#include <stddef.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int use_after(void)
{
  int x = 1;
  dispose(&x);
  return x;
}
