// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (flip A3; was tests/memory/negative/array_reject_invariant_decay.c): an array used as a pointer (q == a) in a loop invariant: q keeps the address of a[0] through the loop
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>

int walk(void)
{
  int a[3] = {1, 2, 3};
  int *q = &a[0];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && q == a)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i += 1;
  }
  return *q;
}
