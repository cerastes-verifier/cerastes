// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 10 (0039 item 4, S44 (i)): an access beside a call whose callee owns a range, and two such calls, in one full expression, on disjoint places: the placement rows (the range forms of item 8's separation) hold
// Pre-registered from record 0060 item 10 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int sumo(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 2)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == 7)
{
  (void)p;
  (void)n;
  return 7;
}

int read_beside(void)
    CERASTES_ENSURES(r, r == 8)
{
  int a[4] = {1, 2, 3, 4};
  int x = a[0] + sumo(&a[1], 2);
  return x;
}

int two_calls(void)
    CERASTES_ENSURES(r, r == 14)
{
  int a[4] = {1, 2, 3, 4};
  int x = sumo(&a[0], 2) + sumo(&a[2], 2);
  return x;
}
