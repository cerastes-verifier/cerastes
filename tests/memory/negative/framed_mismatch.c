// expect: error permission @ L32
// mode: c23, c++20
// twin: tests/memory/framed_guarded.c
// Pins: 0037 item 7: the atom is on c > 0 but the read on c >= 0: the framing fails on the path c == 0 only
#include <cerastes.h>
#include <stddef.h>

int f(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_REQUIRES(p == NULL || *p > 0)
    CERASTES_ENSURES(r, p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, p == NULL || *p == cerastes_old(*p))
    CERASTES_ENSURES(r, r == (p == NULL ? 0 : cerastes_old(*p) - 1))
{
  if (p == NULL) {
    return 0;
  }
  return *p - 1;
}

int use_f(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 5;
  int a = f(&x);
  int b = f(NULL);
  return a + b + x - 5;
}

int matched(int *p, int c)
    CERASTES_REQUIRES(c > 0 ? cerastes_owns(*p) : 1)
    CERASTES_REQUIRES(c >= 0 ? *p > 0 : 1)
    CERASTES_ENSURES(r, c > 0 ? cerastes_owns(*p) : 1)
{
  if (c > 0) {
    return *p;
  }
  return c;
}

int b1_ok(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, p != NULL ? *p == 1 : r == 0)
{
  if (p == NULL) {
    return 0;
  }
  *p = 1;
  return 0;
}

void b2_ok(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q) && *q == 5)
{
  (void)p;
  *q = 5;
}
