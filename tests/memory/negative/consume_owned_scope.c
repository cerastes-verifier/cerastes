// expect: error lifetime @ L15
// mode: c23, c++20
// twin: tests/memory/consume_owned.c
// Pins: 0037 item 13: the callee kept the cell of a local, so the scope exit finds the object not whole: lifetime at the closing brace
#include <cerastes.h>
#include <stddef.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void leak(void)
{
  int x = 1;
  dispose(&x);
}
