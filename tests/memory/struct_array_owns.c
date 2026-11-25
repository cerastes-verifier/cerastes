// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 7 and 8 (flip A of M4.5e1e1, the staging pin struct_array_reject_pointer_subscript.c retired): p->a[i] through a pointer under owns(*p) (f, unchanged, line for line), an element read with old in a contract (bump), and a callee that writes p->a[i] and p->n, called with the address of a structure local
// Pre-registered from record 0063 item 7 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4)
{
  return p->a[i];
}

void bump(struct S *p, int i)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4 && p->a[i] < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[i] == cerastes_old(p->a[i]) + 1 && p->n == cerastes_old(p->n))
{
  p->a[i] += 1;
}

void set(struct S *p, int i, int v)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4)
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[i] == v && p->n == i)
{
  p->a[i] = v;
  p->n = i;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {{1, 2, 3, 4}, 5};
  bump(&s, 1);
  CERASTES_CHECK(s.a[1] == 3 && s.n == 5);
  set(&s, 2, 9);
  CERASTES_CHECK(s.a[2] == 9 && s.n == 2);
  return s.a[2] - 6;
}
