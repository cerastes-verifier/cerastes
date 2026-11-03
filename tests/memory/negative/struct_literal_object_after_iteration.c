// expect: error lifetime @ L25
// c-only: compound-literal objects (&(T){...}) are C only: in C++ the literal is a temporary (decision 0044 item 7)
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (H3): the literal of one iteration is a different object in the next: the pointer kept from the last iteration is compared at the top of the next: lifetime
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
  while (i < 3) {
    if (i > 0 && p == 0) {  // <- here
      return 1;
    }
    p = &(struct S){1, 2};
    i += 1;
  }
  return 0;
}
