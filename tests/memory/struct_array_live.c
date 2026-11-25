// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 6 and 8: a callee that needs the liveness of &s.a[1] and one that needs owns(s.a), each giving it back: the scope exit's rows over the member's bound local hold
// Pre-registered from record 0063 item 6 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void hold(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p));

void lend(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->a))
    CERASTES_ENSURES(cerastes_owns(p->a));

int f(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  hold(&s.a[1]);
  lend(&s);
  return s.n;
}
