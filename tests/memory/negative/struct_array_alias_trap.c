// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_alias.c
// Pins: 0063 item 7 (the alias trap): q is the address of s.a[1], a slot of the array leaf that owns(*p) already holds: the slots overlap, so the call needs the permission twice (the per-slot separation row)
// Pre-registered from record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void two(struct S *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
{
  p->n = 1;
  *q = 2;
}

int use(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  two(&s, &s.a[1]);  // <- here
  return s.n;
}
