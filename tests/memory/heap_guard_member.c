// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 11 (question 109): p->q == NULL || owns(*p->q): a heap guard through a struct member, a callee writing p->q->x, callers with a non-null and a null member
#include <cerastes.h>
#include <stddef.h>

struct M {
  int y;
};

struct L {
  struct M *q;
  int x;
};

void clear(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && (p->q == NULL || cerastes_owns(*p->q)))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && (p->q == NULL || cerastes_owns(*p->q)))
    CERASTES_ENSURES(p->q == NULL || p->q->y == 0)
{
  if (p->q != NULL) {
    p->q->y = 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct M t = {5};
  struct L s = {&t, 1};
  clear(&s);
  CERASTES_CHECK(t.y == 0);
  struct L u = {0, 2};
  clear(&u);
  return t.y;
}
