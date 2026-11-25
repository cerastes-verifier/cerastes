// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 8 (0060 item 8): the decay of an array global passed whole to a callee's reads_range and owns_range parameters: bound to a local first, the shape row and the marked rows hold, the callee's ensures come back
// Pre-registered from record 0063 item 8 (Part B), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

static int g[4] = {1, 2, 3, 4};

int peek(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

void zero_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

int main(void)
{
  int x = peek(g, 4);
  CERASTES_CHECK(x == 1);
  (void)x;
  zero_first(g, 4);
  CERASTES_CHECK(g[0] == 0);
  return 0;
}
