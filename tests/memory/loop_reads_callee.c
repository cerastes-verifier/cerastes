// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3: a callee with reads(*p) does not reach the cell's value (only a callee that owns a cell of its type does), so x keeps 7 across the loop
#include <cerastes.h>

int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p);

int keep_after_peek(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int s = 7;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 7)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = peek(&x);
    i += 1;
  }
  return s;
}
