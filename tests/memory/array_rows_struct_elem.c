// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5 and 7 (Part E; the program of the retired pin array_reject_struct_elem_array_member.c is the declaration of v): an array of structures whose element has an array member, v[i].a[j] (the element's slot, then the member's block, two index steps: two bound rows), in a local at symbolic indexes and through p[i].a[j] under owns_range(p, n) (the contract reads p[i].a[j]), with the elements' own length facts (len == 4 and 1); and a loop invariant that reads local elements' array members, v[e].a[e2] (question 377: accepted for a local, edg-backend report 15's b8_inv_local.c)
// Pre-registered from record 0064 items 3, 5 and 7 (Part E), before any run: verifies
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int local(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 4)
    CERASTES_ENSURES(r, r == 7)
{
  struct W v[2];
  v[i].a[j] = 7;
  return v[i].a[j];
}

void put(struct W *p, int n, int i, int j)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < 4)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i].a[j] == 7)
{
  (void)n;
  p[i].a[j] = 7;
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct W v[2] = {{{0, 0, 0, 0}, 0}, {{0, 0, 0, 0}, 0}};
  put(v, 2, 1, 3);
  return v[1].a[3];
}

int loop_local(void)
    CERASTES_ENSURES(r, r == 2)
{
  [[maybe_unused]] struct W v[2] = {{{1, 2, 3, 4}, 0}, {{5, 6, 7, 8}, 0}};
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && v[1].a[2] == 7 && v[0].a[1] == 2)
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return k;
}
