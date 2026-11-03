// expect: rejected cerastes-unsupported_declaration @ L22
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit B1: an element passes the address of a leaf of the object being initialized to a call: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int g(int *q CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 7)
{
  *q = 6;
  return 7;
}

int self_call(void)
{
  struct S t = {.x = 1, .y = g(&t.x)};  // <- here
  return t.x;
}
