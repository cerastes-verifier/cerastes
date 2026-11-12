// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (flip A2; was tests/memory/negative/array_reject_invariant_pointer_element.c): p[0] of a pointer in a loop invariant, framed by the cell place of owns(*p): the read-only loop keeps it. The program is the old pin's with (void)p; added so that it builds natively (p is used only in the contract)
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>

int count(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10 && *p >= 0)
{
  (void)p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0] >= 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
