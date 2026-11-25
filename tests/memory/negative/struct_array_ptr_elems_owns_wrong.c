// expect: error check @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_ptr_elems_owns.c
// Pins: 0063 items 7 and 15 (audit round 4 S1, the probe k11b): the callee stored &y in element 0: a check that element 0 is still NULL fails (a comparison of an unknown element 1 would add lifetime and pointer-comparison errors)
// Pre-registered from audit round 4 S1 and record 0063 item 15 (Commit 4), before any run: the one error
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
  CERASTES_CHECK(s.a[0] == NULL);  // <- here
  return 0;
}
