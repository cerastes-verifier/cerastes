// expect: error check @ L39
// c-only: the twin of a C-only test
// twin: tests/memory/struct_array_literal.c
// Pins: 0063 item 13: set() writes 9 through the pointer to the literal: a check that p->a[1] is 8 fails
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int get(struct S s)
    CERASTES_REQUIRES(s.a[2] == 3)
    CERASTES_ENSURES(r, r == s.a[2])
{
  return s.a[2];
}

void set(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[1] == 9)
{
  p->a[1] = 9;
}

int by_value(void)
    CERASTES_ENSURES(r, r == 3)
{
  return get((struct S){{1, 2, 3, 4}, 5});
}

int under_address(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct S *p = &(struct S){{1, 2, 3, 4}, 5};
  set(p);
  CERASTES_CHECK(p->a[1] == 8);  // <- here
  return p->a[1];
}
