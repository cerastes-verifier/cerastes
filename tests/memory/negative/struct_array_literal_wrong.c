// expect: error precondition @ L31
// c-only: the twin of a C-only test
// twin: tests/memory/struct_array_literal.c
// Pins: 0063 item 13: the literal's element 2 is 0, not 3: the call `get(...)` fails the precondition s.a[2] == 3
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
  return get((struct S){{1, 2, 0, 4}, 5});  // <- here
}

int under_address(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct S *p = &(struct S){{1, 2, 3, 4}, 5};
  set(p);

  return p->a[1];
}
