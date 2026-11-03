// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/struct_must_init.c
// Pins: 0043 item 9: the callee never writes p->y, which must_init promises: postcondition at the return, naming p->y
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void init(struct S *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->x = 1;
}  // <- here
