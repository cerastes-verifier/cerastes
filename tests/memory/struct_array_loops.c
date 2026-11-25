// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 10: the array leaf of a structure local and of a held structure place of a parameter is an array place of the loop: a fill loop over s.a keeps the permission, the flags and the range (s.n is pinned by the invariant), a write loop over p->a under owns(*p), a read-only loop over p->a under reads(*p) at the atom's amount
// Pre-registered from record 0063 item 10 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 4)
    CERASTES_ENSURES(r, r == n)
{
  struct S s = {{0}, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s.n == 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s.a[i] = i;
    i = i + 1;
  }
  s.n = i;
  return s.n + s.a[0] * 0;
}

void set_all(struct S *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->n == 4)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = v;
    i = i + 1;
  }
  p->n = 4;
}

int largest(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  int m = p->a[0];
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    if (p->a[i] > m) {
      m = p->a[i];
    }
    i = i + 1;
  }
  return m;
}
