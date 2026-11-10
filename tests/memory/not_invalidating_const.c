// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 (question 203): a marked pointer to a const object is accepted (0042 item 4's const rejection is an initialization rule): reads(*p) in and out returns the read share, so the marker is satisfied; the caller writes x after the call
// Pre-registered from record 0058 item 2 (question 203): verifies in both modes
#include <cerastes.h>

int read_it(const int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 4;
  int v = read_it(&x);
  x = 5;
  return v;
}
