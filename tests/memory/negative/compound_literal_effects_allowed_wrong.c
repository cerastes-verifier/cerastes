// expect: error check @ L21
// c-only: compound literals are C (C++ rejects them, record 0044 item 7)
// twin: tests/memory/compound_literal_effects_allowed.c
// Pins: 0063 item 12 (audit round 2, B1): the element is the call's result, the old value 1, which the assignment stores into x after the call raised it to 2: the check that x is 2 fails
// Pre-registered from audit round 2 of record 0063 (B1, question 308), before any run: the one error
#include <cerastes.h>

int bump(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q < 100)
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == cerastes_old(*q) + 1 && r == cerastes_old(*q))
{
  int v = *q;
  *q = v + 1;
  return v;
}

int call_element(void)
{
  int x = 1;
  x = (int[]){bump(&x)}[0];
  CERASTES_CHECK(x == 2);  // <- here
  return x;
}
