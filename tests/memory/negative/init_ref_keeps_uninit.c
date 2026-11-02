// expect: error uninitialized @ L20
// mode: c23, c++20
// twin: tests/memory/init_ref_keeps.c
// Pins: 0042 item 5 (question 87): ref_to_uninit alone does not promise a value: the object was uninitialized before the call and the callee may not write it
#include <cerastes.h>

void maybe(int *p CERASTES_REF_TO_UNINIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 5;
  }
}

int keep(int c)
{
  int x;
  maybe(&x, c);
  return x;  // <- here
}
