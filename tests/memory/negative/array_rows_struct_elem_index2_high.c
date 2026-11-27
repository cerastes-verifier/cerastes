// expect: error out-of-bounds @ L18
// expect: error invariant-entry @ L27
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem.c
// Pins: 0064 item 5 (and, in loop_local, question 377: a loop invariant that reads a local element's array member, v[1].a[2] == 8 where the cell holds 7, does not hold on entry: invariant-entry at the invariant, edg-backend report 15's b8_inv_local_wrong.c): v[i].a[j] with the second index one past its bound reaches no element (the element's own length): out-of-bounds at the first access (the first bound row precedes the second and the cell's), nothing else on the path
// Pre-registered from record 0064 item 5 (Part E), before any run: the one error
#include <cerastes.h>
struct W {
  int a[4];
  int n;
};

int local(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j <= 4)
    CERASTES_ENSURES(r, r == 7)
{
  struct W v[2];
  v[i].a[j] = 7;  // <- here
  return v[i].a[j];
}

int loop_local(void)
    CERASTES_ENSURES(r, r == 2)
{
  [[maybe_unused]] struct W v[2] = {{{1, 2, 3, 4}, 0}, {{5, 6, 7, 8}, 0}};
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && v[1].a[2] == 8 && v[0].a[1] == 2)  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return k;
}
