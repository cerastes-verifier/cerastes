// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 8: == and null tests of struct S * values: the model's identity, p == NULL never checked for pointer-comparison
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

int same(struct S *p, struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == (p == q))
{
  return p == q;
}

int is_null(struct S *p)
    CERASTES_REQUIRES(p == NULL || cerastes_live(p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_live(p)) && r == (p == NULL))
{
  return p == NULL;
}

int use_ops(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s = {1, 2};
  struct S t = {3, 4};
  return same(&s, &t) + is_null(&s) + is_null(NULL);
}
