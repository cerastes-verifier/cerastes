// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/memory/init_loop.c
// Pins: 0042 item 10 (audit N7): a completeness cost, pinned as an expected failure: the loop always runs and writes *p, but it keeps only ds ==> flag, so the flag row at the return fails; writing before the loop, or a written atom in the invariant, is the workaround (record 0038 says the same for locals)
#include <cerastes.h>

void fill(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *p = i;
    i += 1;
  }
}  // <- here
