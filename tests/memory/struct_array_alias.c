// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 7 (overlap is by slots): owns(*p) and owns(*q) with p the structure and q another object: disjoint, the call holds
// Pre-registered from record 0063 item 7 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void two(struct S *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q) && p->n == 1 && *q == 2)
{
  p->n = 1;
  *q = 2;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int other = 0;
  two(&s, &other);
  CERASTES_CHECK(s.n == 1 && other == 2);
  return s.n + other;
}
