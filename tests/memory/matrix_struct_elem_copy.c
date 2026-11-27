// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 323; S48 (g): the guarded copy took 35 s in the spike, so this test is timed under the budget's rule 3): v[i] = v[j] of elements with an array member copies the cell leaf by a store and the array leaf by the trusted copy under the guard d != s (two elements of one array may be one), and after it the two elements agree whatever i and j are
// Pre-registered from record 0064 item 12 (Part E), before any run: verifies (time it first)
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int copy(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, r == 0)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  v[i] = v[j];
  return (v[i].a[2] - v[j].a[2]) + (v[i].n - v[j].n);
}
