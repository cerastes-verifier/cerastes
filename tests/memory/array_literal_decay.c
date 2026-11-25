// expect: verified
// expect: native-ok
// mode: c23
// c-only: array compound literals are C only
// Pins: 0063 item 12: the decay of an array compound literal: in an initializer (int *q = (int[]){...}, then q[i] in a check and a return) and as an argument (a reads_range callee takes the hidden array whole, and the value it returns is the literal's first element)
// Pre-registered from record 0063 item 12 (Part D), before any run: verifies
#include <cerastes.h>

int first(const int *p)
    CERASTES_REQUIRES(cerastes_reads_range(p, 3))
    CERASTES_ENSURES(r, cerastes_reads_range(p, 3) && r == p[0])
{
  return p[0];
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int *q = (int[]){1, 2, 3};
  int x = first((int[]){4, 5, 6});
  CERASTES_CHECK(x == 4);
  CERASTES_CHECK(q[2] == 3);
  return q[0] + x - 4;
}
