// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5: two written atoms on two distinct owned places, both written in the loop
#include <cerastes.h>
#include <stddef.h>

void two(int *p, int *r, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*r) && 0 <= n && n <= 10)
{
  int *a = p;
  int *b = r;
  p = a;
  r = b;
  int *q = p;
  int *s = r;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*q) && cerastes_owns(*s))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *q = i;
    *s = i;
    i += 1;
  }
}
