// expect: error lifetime @ L26
// c-only: compound-literal objects (&(T){...}) are C only: in C++ the literal is a temporary (decision 0044 item 7)
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit S1): a pointer to the literal of a switch's controlling expression compared after the switch: lifetime
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->x = 7;
}

int f(void)
{
  struct S *p = 0;
  switch ((p = &(struct S){1, 2}) != 0) {
  default:
    break;
  }
  return p == 0;  // <- here
}
