// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 11: s.a[0] = s.a[1]++ over two constant elements of an array member is sequenced enough (distinct slots), and p->a[i] = p->a[j]++ under i != j
// Pre-registered from record 0063 item 11 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int shift(struct S *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 4 && 0 <= j && j < 4 && i != j && 0 <= p->a[j] && p->a[j] < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p->a[j])
{
  p->a[i] = p->a[j]++;
  return p->a[j];
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {{1, 2, 3, 4}, 5};
  s.a[0] = s.a[1]++;
  CERASTES_CHECK(s.a[0] == 2 && s.a[1] == 3);
  return s.a[1];
}
