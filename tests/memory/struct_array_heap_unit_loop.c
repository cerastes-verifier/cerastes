// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 10 (audit round 3 S4, the probe c05): a unit that calls malloc elsewhere, a loop and a pointer local to a structure with an array member: verifies; the loop's heap holders do not take the array leaf as a cell (today exit 3 with no diagnostic) (twin negative/struct_array_heap_unit_loop_wrong.c)
// Pre-registered from audit round 3 S4 and record 0063 item 10 (Part C), before any run: verifies (today exit 3: internal error in the front end, no diagnostic)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int g(void)
{
  int *h = (int *)malloc(sizeof(int));
  if (h == NULL) {
    return 0;
  }
  *h = 1;
  int v = *h;
  free(h);
  return v - v;
}

int main(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    p->a[1] = i;
    i = i + 1;
  }
  CERASTES_CHECK(i == 3);
  return g();
}
