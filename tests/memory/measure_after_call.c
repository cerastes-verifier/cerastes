// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 8 and 0021 item 4: the recursive call follows another call in its expression, so the explicit decrease check is printed: it reads the measure of the callee over the current heap and the caller's under old
#include <cerastes.h>
#include <stddef.h>

int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int down(int *p, const int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_reads(*q) && r == 0)
{
  if (*p == 0) {
    return 0;
  }
  *p -= 1;
  return peek(q) * 0 + down(p, q);
}
