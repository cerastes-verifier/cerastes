// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 7 and 15 (audit round 4 S1, the probe k11): an array member of POINTER elements under owns(*p): the contract's quantified range rows over the member are the pointer's WF (null, or its own block with 0 <= offset <= len), which the lint accepts beside the integer range (twin negative/struct_array_ptr_elems_owns_wrong.c)
// Pre-registered from audit round 4 S1 and record 0063 item 15 (Commit 4), before any run: verifies (the lint refused the pointer WF range of the member's rows)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int *a[2];
  int n;
};

void set(struct S *p, int *x)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_live(x))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_live(x) && p->a[0] == x)
{
  p->a[0] = x;
}

int main(void)
{
  int y = 0;
  struct S s = {{NULL, NULL}, 0};
  set(&s, &y);
  CERASTES_CHECK(s.a[0] == &y);
  return 0;
}
