// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3 and 5 (Part E; depth 3 as a member of a structure: t.v[i].a[j] has the member's index, the element's member and its index): a local structure with a member array of structures whose elements have an array member, written and read at symbolic indexes (a structure local with such a member costs about 45 s: rule 3, timed)
// Pre-registered from record 0064 items 3 and 5 (Part E), before any run: verifies (time it first)
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 4)
    CERASTES_ENSURES(r, r == 7)
{
  struct T t;
  t.k = 0;
  t.v[i].a[j] = 7;
  return t.v[i].a[j];
}
