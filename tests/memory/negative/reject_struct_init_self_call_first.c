// expect: rejected cerastes-unsupported_declaration @ L22
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 audit B1: a designated element passes the address of another leaf of the object to a call: rejected
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
  struct S s = {.x = g(&s.y)};  // <- here
  return s.y;
}
