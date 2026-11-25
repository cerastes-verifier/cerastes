// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 7 and 8 (flip A of M4.5e1e1, the staging pin struct_array_reject_owns_member.c retired): owns(p->a) and reads(p->a), the whole array member as the atom: the declaration and f as the pin had them (the body now uses p), a callee that reads and one that writes the array, and a caller whose other member s.n is not given away
// Pre-registered from record 0063 item 7 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->a))
    CERASTES_ENSURES(cerastes_owns(p->a))
{
  (void)p;
}

int first(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(p->a))
    CERASTES_ENSURES(r, cerastes_reads(p->a) && r == p->a[0])
{
  return p->a[0];
}

void zero_first(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->a))
    CERASTES_ENSURES(cerastes_owns(p->a) && p->a[0] == 0)
{
  p->a[0] = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int v = first(&s);
  zero_first(&s);
  CERASTES_CHECK(v == 1 && s.a[0] == 0 && s.n == 5);
  return v - 1 + s.a[0];
}
