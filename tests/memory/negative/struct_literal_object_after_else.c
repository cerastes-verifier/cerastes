// expect: error lifetime @ L26
// c-only: compound-literal objects (&(T){...}) are C only: in C++ the literal is a temporary (decision 0044 item 7)
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit S1): a pointer to the literal of an else arm (a block of its own) compared after the if: lifetime
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

int f(int c)
{
  struct S *p = 0;
  if (c != 0) {
    p = 0;
  } else
    p = &(struct S){1, 2};
  return p == 0;  // <- here
}
