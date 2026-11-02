// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 5 (audit S1, question 89): a loop that touches memory (it reads *q) but does not reach a place (the cell x in after_loop, *p in spin) keeps held && flag ==> range for it, so the marked call after the loop and the return rows of a ref_to_uninit function verify
#include <cerastes.h>

void leave(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

int after_loop(int *q, int n)
    CERASTES_REQUIRES(cerastes_reads(*q) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_reads(*q))
{
  int x;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*q > 0) {
      s = 1;
    }
    i += 1;
  }
  leave(&x);
  return s;
}

void spin(int *p CERASTES_REF_TO_UNINIT, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_reads(*q))
{
  int i = 0;
  (void)p;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += (*q > 0) ? 1 : 1;
  }
}
