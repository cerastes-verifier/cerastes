// expect: rejected cerastes-unsupported_expression @ L16
// mode: c23, c++20
// twin: tests/memory/array_ptr_param_first.c
// Pins: 0059 item 10: p[i].x on a pointer to a structure is an array of structures, rejected until M4.5e1e
// Pre-registered from record 0059 item 10 (Part D): the tag is my reading (an element access that is not an element of a local array)
#include <cerastes.h>

struct S {
  int x;
};

int get(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p[i].x;
}
