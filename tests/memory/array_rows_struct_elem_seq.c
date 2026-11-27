// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E): the object keys of v[i].a[j] carry the element's index, the member and the member's index: v[i].a[j] = v[k].a[l]++ is sequenced enough under (i != k || j != l) (two elements, or two cells of one); the twin removes the distinctness
// Pre-registered from record 0064 item 11 (Part E), before any run: verifies
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int cells(int i, int j, int k, int l)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 4 && 0 <= k && k < 2 && 0 <= l && l < 4 && (i != k || j != l))
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  int old = v[k].a[l];
  v[i].a[j] = v[k].a[l]++;
  CERASTES_CHECK(v[i].a[j] == old && v[k].a[l] == old + 1);
  return v[i].a[j] - old;
}
