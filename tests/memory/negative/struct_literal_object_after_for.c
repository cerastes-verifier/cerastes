// expect: error lifetime @ L26
// c-only: compound-literal objects (&(T){...}) are C only: in C++ the literal is a temporary (decision 0044 item 7)
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (H3, audit S1): a pointer to the literal of a for init compared after the for: lifetime
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
  int i = 0;
  CERASTES_DECREASES(3 - i)
  for (p = &(struct S){1, 2}; i < 3; i += 1) {
  }
  return p == 0;  // <- here
}
