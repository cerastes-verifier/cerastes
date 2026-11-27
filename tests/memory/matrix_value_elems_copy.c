// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, the second type of the "tested so" pair): the whole copy w = u of struct T { struct W v[2]; int k; } with struct W { int a[2]; int n; } writes every leaf of every element of v (six cells, in the value view's order) and k: u holds a distinct value in every cell and every cell of w is read back by a check in code; negative/matrix_value_elems_copy_wrong.c is the twin
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): verifies (time it first: two struct T locals in one function, 59 to 61 s on the loaded box by record 0064 item 12's cost note, so the file may be slimmed to one function)
#include <cerastes.h>

struct W {
  int a[2];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int copy_cells(void)
    CERASTES_ENSURES(r, r == 28)
{
  struct T u = {{{{1, 2}, 3}, {{4, 5}, 6}}, 7};
  struct T w = {{{{0, 0}, 0}, {{0, 0}, 0}}, 0};
  w = u;
  CERASTES_CHECK(w.v[0].a[0] == 1);
  CERASTES_CHECK(w.v[0].a[1] == 2);
  CERASTES_CHECK(w.v[0].n == 3);
  CERASTES_CHECK(w.v[1].a[0] == 4);
  CERASTES_CHECK(w.v[1].a[1] == 5);
  CERASTES_CHECK(w.v[1].n == 6);
  CERASTES_CHECK(w.k == 7);
  return w.v[0].a[0] + w.v[0].a[1] + w.v[0].n + w.v[1].a[0] + w.v[1].a[1] + w.v[1].n + w.k;
}
