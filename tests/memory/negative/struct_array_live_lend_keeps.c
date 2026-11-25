// expect: error lifetime @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_live.c
// Pins: 0063 items 6 and 8: a callee that keeps the array member (owns(p->a) in its requires, nothing back): the scope exit finds the array leaf not whole (lifetime at the return)
// Pre-registered from record 0063 item 6 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void lend(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->a));

int f(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  lend(&s);
  return s.n;  // <- here
}
