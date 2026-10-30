// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 4: two memory calls C does not order: bump(&x) + bump(&y) and bump(&x) + peek(&y) (a reads callee) name different cells, so arg1 != arg2 holds
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int two(void)
    CERASTES_ENSURES(r, r == 11)
{
  int x = 1;
  int y = 2;
  int s = bump(&x) + bump(&y);
  int t = bump(&x) + peek(&y);
  return s + t;
}
