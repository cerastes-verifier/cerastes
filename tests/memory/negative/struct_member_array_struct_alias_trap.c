// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_alias.c
// Pins: 0064 items 3 and 7 (the alias trap): q is the address of t.v[1], an element of the array member that owns(*p) already holds: the slots overlap, so the call needs the permission twice (the per-leaf separation row)
// Pre-registered from record 0064 item 7 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

void two(struct T *p, struct P *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
{
  p->n = 1;
  q->x = 2;
}

int use(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  two(&t, &t.v[1]);  // <- here
  return t.n;
}
