// expect: error check @ L29
// expect: error check @ L45
// mode: c23, c++20
// twin: tests/memory/struct_pointer_array_member.c
// Pins: 0064 item 3 (audit round C2, finding 1, the array-of-pointers family): set_m checks that the int member is 2 after writing 1; zero_all checks that t.n is 6 after the loop that only zeroes the pointers (t.n is 5). One reason each: a wrong value claim
// Registered from il-analyst-report-11 (c) (w8f: `check(p->m == 2)` measured: check; w4f claimed a pointer value and drew `lifetime` and `pointer-comparison` beside the check, so this one claims t.n instead, unmeasured), before the first run on the build with the fix; the rows' lines are inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct H2 {
  struct T *ptrs[2];
  int m;
};

int set_m(struct H2 *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  p->m = 1;
  CERASTES_CHECK(p->m == 2);  // <- here
  return 0;
}

int zero_all(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  [[maybe_unused]] struct T *a[2] = {&t, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    a[i] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(t.n == 6);  // <- here
  return t.n;
}
